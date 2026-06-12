#ifndef _DEVICES_H_
#define _DEVICES_H_

// IIDX_DEVICE_HEADER で指定されたヘッダファイルをインクルード
// CMake から IIDX_DEVICE_HEADER="devices/rainbow2plus.h" などが渡される
#ifndef IIDX_DEVICE_HEADER
#define IIDX_DEVICE_HEADER "devices/iidx_ps2.h"
#endif 

#include IIDX_DEVICE_HEADER

#endif // _DEVICES_H_
