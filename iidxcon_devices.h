#ifndef _IIDXCON_DEVICES_H_
#define _IIDXCON_DEVICES_H_

// デバイス指定は CMake からマクロ定義で渡される想定
// (例: -DIIDX_DEVICE=RAINBOW2 を CMake 引数に指定)
// いずれのマクロも指定されていない場合は IIDX_PS2 をデフォルトにする
#if !defined(RAINBOW2PLUS) && !defined(RAINBOW2) && !defined(DAO_FPS) && !defined(IIDX_PS2)
#  define IIDX_PS2
#  warning "IIDX_DEVICE not specified; defaulting to IIDX_PS2"
#endif

// 各デバイスの設定をインクルード
#if defined(RAINBOW2PLUS)
#  include "devices/rainbow2plus.h"
#elif defined(RAINBOW2)
#  include "devices/rainbow2.h"
#elif defined(DAO_FPS)
#  include "devices/dao_fps.h"
#elif defined(IIDX_PS2)
#  include "devices/ps2.h"
#endif

#endif // _IIDXCON_DEVICES_H_
