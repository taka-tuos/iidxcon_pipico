# iidxcon_pipico

## 特徴
- GPLv3
  - 人類に打鍵の自由を！
- エントリーコン偽装
- PS2対応(WIP)

## ビルド方法

```bash
git clone https://github.com/taka-tuos/iidxcon_pipico
cd iidxcon_pipico
mkdir build
cd build
export PICO_SDK_PATH=../../pico-sdk
cmake ..
make -j4
```

### ボード指定

`cmake` 実行時に `IIDX_DEVICE` を指定できます。書きこむ基板に合わせて選択してください。

有効な値:
- `IIDX_PS2` (PS2専コン、デフォルト)
- `RAINBOW2PLUS` (Rainbow Controller 2 Plus)
- `RAINBOW2` (Rainbow Controller 2)
- `DAO_FPS` (DAO FPS)

例:

```bash
cmake -DIIDX_DEVICE=RAINBOW2 ..
```

指定がない場合は `IIDX_PS2` が使用され、CMake設定時に警告が表示されます。
