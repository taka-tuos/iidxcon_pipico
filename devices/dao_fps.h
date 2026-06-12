#ifndef _DEVICES_DAO_FPS_H_
#define _DEVICES_DAO_FPS_H_

#include "../devices_common.h"

// DAO FPS 用基板
const int keys[KEY_COUNT] = {
	BTN_DISABLED, BTN_DISABLED, BTN_DISABLED, BTN_DISABLED, BTN_DISABLED, BTN_DISABLED, BTN_DISABLED,
	BTN_DISABLED, BTN_DISABLED, BTN_DISABLED, BTN_DISABLED
};

const int scr[SCR_COUNT] = {
	BTN_DISABLED, BTN_DISABLED
};

const bool psx_enable = false;
const int psx_att = -1; // CS
const int psx_sck = -1; // SCK
const int psx_cmd = -1; // MOSI
const int psx_dat = -1; // MISO
const int psx_ack = -1; // ACK

#endif // _DEVICES_DAO_FPS_H_
