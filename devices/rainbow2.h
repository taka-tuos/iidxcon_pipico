#ifndef _IIDXCON_RAINBOW2_H_
#define _IIDXCON_RAINBOW2_H_

// Rainbow2 用基板
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

#endif // _IIDXCON_RAINBOW2_H_
