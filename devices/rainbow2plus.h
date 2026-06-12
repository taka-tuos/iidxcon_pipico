#ifndef _IIDXCON_RAINBOW2PLUS_H_
#define _IIDXCON_RAINBOW2PLUS_H_

// Rainbow2Plus 用基板
// これだけ E3 がある
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

#endif // _IIDXCON_RAINBOW2PLUS_H_
