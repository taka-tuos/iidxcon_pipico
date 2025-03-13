/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * Modification of dev_hid_composite example to create a simple joystick.
 * 2 axes and 8 buttons. Compatible with the Xbox Adaptive Controller.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "hardware/gpio.h"

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

#pragma region 定数等

// デバイス指定 どれか一つコメントアウトしてね！
#define RAINBOW2PLUS
//#define RAINBOW2
//#define IIDX_PS2

#ifdef IIDX_PS2
// PS2専コン用基板
const int keys[9] = {
	5,2,8,7,6,9,10, // 1-7
	4,3,-1,-1       // START,SELECT,E3,E4
};

const int scr[2] = {
	11,12
};
#endif

#ifdef RAINBOW2PLUS
// Rainbow2Plus用基板
// これだけE3がある
const int keys[11] = {
	26,13,27,14,28,15,29,
	10,11,12,-1
};

const int scr[2] = {
	8,9
};
#endif

#ifdef RAINBOW2
// Rainbow2用基板
const int keys[11] = {
	13,14,15,26,27,28,29,
	10,9
};

const int scr[2] = {
	11,12
};
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
		if(keys[i] != -1) {
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

	// 1ms以上経ってたら…
	// TODO: これいらんくね？
	if ((board_millis() - start_ms) < interval_ms) return; // not enough time
	start_ms = board_millis() + interval_ms;
	
	// Remote wakeup
	if (tud_suspended()) {
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	}

	// Reportを初期化
	report.xAxis = 0;
	report.yAxis = 0;
	report.buttons[0] = 0;
	report.buttons[1] = 0;
	report.buttons[2] = 0;
	
	// デバウンスしながら埋める
	for(int i = 0; i < 11; i++) {
		if(keys[i] != -1) {
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
	if (tud_hid_ready()) {
		tud_hid_report(0x00, &report, sizeof(report));
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