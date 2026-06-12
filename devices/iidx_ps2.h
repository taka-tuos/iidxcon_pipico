#ifndef _IIDXCON_IIDX_PS2_H_
#define _IIDXCON_IIDX_PS2_H_

// PS2 専コン用基板
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

#endif // _IIDXCON_IIDX_PS2_H_
