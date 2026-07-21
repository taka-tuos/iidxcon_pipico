#ifndef _DEVICES_DAO_FPS_H_
#define _DEVICES_DAO_FPS_H_

#include "../devices_common.h"

// DAO FPS 用基板
const int keys[KEY_COUNT] = {
	4, 8, 5, 9, 6, 10, 7,
	2, 13, BTN_DISABLED, BTN_DISABLED
};

const int scr[SCR_COUNT] = {
	11, 12
};

const bool psx_enable = true;
const int psx_att = 27; // CS
const int psx_sck = 26; // SCK
const int psx_cmd = 28; // MOSI
const int psx_dat = 29; // MISO
const int psx_ack = 15; // ACK

#endif // _DEVICES_DAO_FPS_H_
