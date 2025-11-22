#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "hardware/gpio.h"
#include "pico/multicore.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms	 : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 1000,
	BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);
void core1_task(void);

#pragma region 定数等

// デバイス指定はCMakeからマクロ定義で渡される想定
// (例: -DIIDX_DEVICE=RAINBOW2 をCMake引数に指定)
// いずれのマクロも指定されていない場合はIIDX_PS2をデフォルトにする
#if !defined(RAINBOW2PLUS) && !defined(RAINBOW2) && !defined(IIDX_PS2)
#  define IIDX_PS2
#  warning "IIDX_DEVICE not specified; defaulting to IIDX_PS2"
#endif

#ifdef IIDX_PS2
// PS2専コン用基板
const int keys[11] = {
	5,2,8,7,6,9,10, // 1-7
	4,3,255,255     // START,SELECT,E3,E4
};

const int scr[2] = {
	11,12
};

const bool psx_enable = true;
const int psx_att = 15; // CS
const int psx_sck = 26; // SCK
const int psx_cmd = 27; // MOSI
const int psx_dat = 28; // MISO
const int psx_ack = 29; // ACK
#endif

#ifdef RAINBOW2PLUS
// Rainbow2Plus用基板
// これだけE3がある
const int keys[11] = {
	26,13,27,14,28,15,29,
	10,11,12,255
};

const int scr[2] = {
	8,9
};

const bool psx_enable = false;
const int psx_att = -1; // CS
const int psx_sck = -1; // SCK
const int psx_cmd = -1; // MOSI
const int psx_dat = -1; // MISO
const int psx_ack = -1; // ACK

#endif

#ifdef RAINBOW2
// Rainbow2用基板
const int keys[11] = {
	13,14,15,26,27,28,29,
	10,9,255,255
};

const int scr[2] = {
	11,12
};

const bool psx_enable = false;
const int psx_att = -1; // CS
const int psx_sck = -1; // SCK
const int psx_cmd = -1; // MOSI
const int psx_dat = -1; // MISO
const int psx_ack = -1; // ACK
#endif

// Reportの何バイト目か
const int map1[11] = {
	0,0,0,0,0,0,0,
	1,1,1,1
};

// 何bit目か
const int map2[11] = {
	0,1,2,3,4,5,6,
	0,1,2,3
};

// PSコンの何バイト目か
const int psx_map1[9] = {
	1,1,1,1,1,1,0,
	0,0
};

// 何bit目か
const int psx_map2[9] = {
	7,2,6,3,5,4,7,
	3,0
};

// 我らの聖典ps_jpn.txtを崇めよ！

#pragma endregion

#pragma region グローバル変数

// デバウンス用
int debounce_timer[11] = {
	0,0,0,0,0,0,0,
	0,0,0,0
};

int debounce_buffer[11] = {
	0,0,0,0,0,0,0,
	0,0,0,0
};

uint8_t psx_buffer[2] = {
	0xff, 0xff
};

// デバウンス時間
#define DEBOUNCE_DURTITION 20

// 0: アナログ
// 1: デジタル
int scr_mode = 0;

// スクラッチ速度
int scr_delta = 1;

#pragma endregion

#pragma region メイン関数
/*------------- MAIN -------------*/
int main(void) {
	board_init();
	tusb_init();
	
	// 全ピン舐めて設定
	for(int i = 0; i < 11; i++) {
		if(keys[i] != 255) {
			gpio_init(keys[i]);
			gpio_set_dir(keys[i], GPIO_IN);
			gpio_pull_up(keys[i]);
		}
	}

	gpio_init(scr[0]);
	gpio_set_dir(scr[0], GPIO_IN);
	gpio_pull_up(scr[0]);
	
	gpio_init(scr[1]);
	gpio_set_dir(scr[1], GPIO_IN);
	gpio_pull_up(scr[1]);

	if(psx_enable) {
		// 入力
		gpio_init(psx_sck);
		gpio_set_dir(psx_sck, false);
		gpio_init(psx_att);
		gpio_set_dir(psx_att, false);
		gpio_init(psx_cmd);
		gpio_set_dir(psx_cmd, false);

		// オープンドレイン
		gpio_init(psx_dat);
		gpio_set_dir(psx_dat, false);
		gpio_put(psx_dat, false);

		gpio_init(psx_ack);
		gpio_set_dir(psx_ack, false);
		gpio_put(psx_ack, false);

		// 出力
		// なんと、ない
	}

	// ピンの状態を安定させる
	{
		static uint32_t start_ms = 0;

		while((board_millis() - start_ms) < 100);
	}
	
	// SELECT押しながらでLR2モード
	scr_mode = !gpio_get(keys[8]) ? 1 : 0;
	
	// 2鍵押してたら倍スクラッチ
	scr_delta = !gpio_get(keys[1]) ? 2 : 1;

	// 4鍵押してたら4倍スクラッチ
	scr_delta = !gpio_get(keys[3]) ? 4 : scr_delta;

	// 6倍は実装予定なし

	// PS用タスクを起動
	multicore_launch_core1(core1_task);

	// ぶんまわし
	while (1) {
		tud_task(); // tinyusb device task
		led_blinking_task();
		hid_task();
	}

	return 0;
}

#pragma endregion

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

#pragma region 謎1

// Invoked when device is mounted
void tud_mount_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us	to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
	(void) remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

#pragma endregion

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+
#pragma region デジタルスクラッチ

int digi_sc = 1;
int prev_sc = 0;
int digi_ctimer = 0;
int digi_vtimer = 0;

uint8_t ana_sc = 0;
uint8_t prev_a = 255;

void scr_check() {
	if(ana_sc != prev_sc) {
		if(ana_sc > prev_sc) {
			if(ana_sc - prev_sc > 127) {
				digi_sc = 0;
			} else {
				digi_sc = 2;
			}
		} else {
			if(prev_sc - ana_sc > 127) {
				digi_sc = 2;
			} else {
				digi_sc = 0;
			}
		}
		digi_vtimer = 0;
	} else {
		if(digi_vtimer >= 150) {
			digi_sc = 1;
		} else {
			digi_vtimer++;
		}
	}
	
	prev_sc = ana_sc;
}

#pragma endregion

#pragma region 本処理

void hid_task(void) {
	const uint32_t interval_ms = 1;
	static uint32_t start_ms = 0;
	static HID_JoystickReport_Data_t report;

	// Reportを初期化
	report.xAxis = 0;
	report.yAxis = 0;
	report.buttons[0] = 0;
	report.buttons[1] = 0;
	report.buttons[2] = 0;
	
	// デバウンスしながら埋める
	for(int i = 0; i < 11; i++) {
		if(keys[i] != 255) {
			// よむ
			int dat = gpio_get(keys[i]);

			// デバウンス時間以上経ってからじゃないと状態変化しない
			if(dat != debounce_buffer[i] && board_millis() - debounce_timer[i] >= DEBOUNCE_DURTITION) {
				debounce_buffer[i] = dat;
				debounce_timer[i] = board_millis();
			}

			// Reportに突っ込む
			report.buttons[map1[i]] |= debounce_buffer[i] ? 0 : (1 << map2[i]);
		}
	}

	// A相とB相
	uint8_t now_a = !gpio_get(scr[1]) ? 1 : 0;
	uint8_t now_b = !gpio_get(scr[0]) ? 1 : 0;
	
	// 前回のA相の値が存在してほしい
	if(prev_a != 255) {
		// A相が変化した時(両相で見てる)
		if(now_a != prev_a) {
			// A相 == B相なら正転
			if(now_a == now_b) {
				// スタート押してるときは半速にする(サドプラ)
				if(!gpio_get(keys[7])) ana_sc++;
				else ana_sc += scr_delta;
			}
			// でなければ…
			if(now_a != now_b) {
				if(!gpio_get(keys[7])) ana_sc--;
				else ana_sc -= scr_delta;
			}
		}
	}
	
	// 前回のA相の値を更新
	prev_a = !gpio_get(scr[1]) ? 1 : 0;

	// デジタル皿チェック！
	if(board_millis() - digi_ctimer > 1) {
		digi_ctimer = board_millis();
		scr_check();
	}

	// 皿モードに応じて突っ込む
	if(!scr_mode) {
		report.xAxis = ((int)ana_sc - 128);
	} else {
		report.yAxis = digi_sc == 1 ? 0 : digi_sc == 2 ? 127 : -128;
	}

	/*------------- Joystick -------------*/
	// 1ms以上経ってたら…
	if ((board_millis() - start_ms) < interval_ms) return; // not enough time
	start_ms = board_millis() + interval_ms;
	
	// Remote wakeup
	if (tud_suspended()) {
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	}

	if (tud_hid_ready()) {
		tud_hid_report(0x00, &report, sizeof(report));
	}
}

#pragma endregion

#pragma region PSコン処理

// redditの仙人に感謝
enum transfer_state {
	TRANSFER_STATE_IDLE,
	TRANSFER_STATE_AWAITING_COMMAND,
	TRANSFER_STATE_SENDING_DATA,
	TRANSFER_STATE_FINISHING,
} state;

// 0: 立下り
void psx_waitedge(int edge) {
	if(edge == 0) {
		while(gpio_get(psx_sck) == 1);
	} else if(edge == 1) {
		while(gpio_get(psx_sck) == 0);
	}
}

// 綴りはわざと
// mode 3, LSB first
uint8_t psx_transfur(uint8_t send) {
	uint8_t dat = 0;

	for(int i = 0; i < 8; i++) {
		// 最初に立下りが来る
		psx_waitedge(0);

		// オープンドレイン駆動
		gpio_set_dir(psx_dat, !(send & (1 << i)));

		// 立ち上がり待ち
		psx_waitedge(0);

		// 読んで入れる
		dat |= ((gpio_get(psx_cmd) ? 1 : 0) << i);
	}

	// Hi-Zに戻す
	gpio_set_dir(psx_dat, false);
}

// パッド情報を組む
void psx_build() {
	psx_buffer[0] = 0xff;
	psx_buffer[1] = 0xff;

	// ボタン
	for(int i = 0; i < 9; i++) {
		if(debounce_buffer[i]) {
			psx_buffer[psx_map1[i]] &= (1 << psx_map2[i]) ^ 0xff;
		}
	}

	// スクラッチ
	if(digi_sc == 1) psx_buffer[0] &= (1 << 4) ^ 0xff;
	else if(digi_sc == 2) psx_buffer[0] &= (1 << 6) ^ 0xff;
}

// Picoには2コア目があってぇ
// SPIペリフェラルを使う→割り込みはCore0だけしか受け取れないのでUSBとぶつかる
// PIO→めんどい
// 雑ポーリング→かんたん(4MHzぐらいまでなら行けてしまうので)
void core1_task() {
	// PS用インタフェースが有効じゃない場合、死ぬ
	if(!psx_enable) return;

	uint8_t data_to_send = 0;
	uint8_t send = 0xff;

	volatile int state = TRANSFER_STATE_IDLE;

	// ぶん回すぜ！
	while(1) {
		// CSがLOWになるまで待つ
		while(gpio_get(psx_att) == 1) state = TRANSFER_STATE_IDLE;

		int ack = 1;
		
		uint8_t data = psx_transfur(send);
		uint8_t command = 0;

		// コピペ元: http://benryves.com/bin/playstation-arcade-stick.zip
		switch(state) {
			case TRANSFER_STATE_IDLE:
				switch (data) {
					case 0x01:
						state = TRANSFER_STATE_AWAITING_COMMAND;
						send = 0x41; // Digital pad.
						break;
					default:
						state = TRANSFER_STATE_IDLE;
						ack = 0;
						break;
				}
				break;
			case TRANSFER_STATE_AWAITING_COMMAND:
				command = data;
				switch (command) {
					case 0x42:
						state = TRANSFER_STATE_SENDING_DATA;
						data_to_send = 2; // 2 bytes of data.
						send = 0x5A; // Data coming.
						break;
					default:
						state = TRANSFER_STATE_IDLE;
						ack = 0;
						break;
				}
				break;
			case TRANSFER_STATE_SENDING_DATA:
				switch (2 - data_to_send) {
					case 0:
						// 1バイト目
						send = psx_buffer[0];
						break;
					case 1:
						// 2バイト目
						send = psx_buffer[1];
						break;
				}
				if (--data_to_send == 0) {
					state = TRANSFER_STATE_FINISHING;
				}
				break;
			case TRANSFER_STATE_FINISHING:
				state = TRANSFER_STATE_IDLE;
				ack = 0;
				break;
		}

		if(ack && !gpio_get(psx_att)) {
			// 5us待って
			uint64_t start = time_us_64();
			psx_build();
			while(time_us_64() - start < 5);

			// LOWにして
			gpio_set_dir(psx_att, true);

			// 2us待って
			start = time_us_64();
			while(time_us_64() - start < 2);

			// HIGHにする
			gpio_set_dir(psx_att, false);

			// マイクロ秒は32bitだと72分でラップアラウンドするので狂う
			// 64bitなら30万年
		} else {
			state = TRANSFER_STATE_IDLE;
			while(!gpio_get(psx_att));
		}
	}
}

#pragma endregion

#pragma region 謎2

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	// TODO not Implemented
	(void) instance;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) reqlen;

	return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
	// TODO set LED based on CAPLOCK, NUMLOCK etc...
	(void) instance;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
	static uint32_t start_ms = 0;
	static bool led_state = false;

	// Blink every interval ms
	if ((board_millis() - start_ms) < blink_interval_ms) return; // not enough time
	start_ms = board_millis() + blink_interval_ms;

	board_led_write(led_state);
	led_state = 1 - led_state; // toggle
}

#pragma endregion
