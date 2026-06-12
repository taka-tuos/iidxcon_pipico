#ifndef _DEVICES_RAINBOW2PLUS_H_
#define _DEVICES_RAINBOW2PLUS_H_

#include "../devices_common.h"

// Rainbow Controller 2Plus 用基板
// これだけE3(VEFX)がある
const int keys[KEY_COUNT] = {
	26, 13, 27, 14, 28, 15, 29,
	10, 11, 12, BTN_DISABLED
};

const int scr[SCR_COUNT] = {
	8, 9
};

const bool psx_enable = false;
const int psx_att = -1; // CS
const int psx_sck = -1; // SCK
const int psx_cmd = -1; // MOSI
const int psx_dat = -1; // MISO
const int psx_ack = -1; // ACK

#endif // _DEVICES_RAINBOW2PLUS_H_
