#ifndef _DEVICES_RAINBOW2_H_
#define _DEVICES_RAINBOW2_H_

#include "../devices_common.h"

// Rainbow Controller 2 用基板
const int keys[KEY_COUNT] = {
	13, 14, 15, 26, 27, 28, 29,
	10, 9, BTN_DISABLED, BTN_DISABLED
};

const int scr[SCR_COUNT] = {
	11, 12
};

const bool psx_enable = false;
const int psx_att = -1; // CS
const int psx_sck = -1; // SCK
const int psx_cmd = -1; // MOSI
const int psx_dat = -1; // MISO
const int psx_ack = -1; // ACK

#endif // _DEVICES_RAINBOW2_H_
