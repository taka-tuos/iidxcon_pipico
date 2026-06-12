# デバイス設定ファイル

このフォルダには、各デバイス固有の設定ヘッダファイルを配置します。

## ファイル名の制約

- **拡張子は `.h` のみ**（例：`my_device.h`）
- **ファイル名は英数字とアンダースコアのみ**を使用してください（例：`my_device`、`MYDEVICE`）
  - 特殊文字やスペース、ハイフンなどは避けてください
- **ファイル名は自動的に大文字に変換**されてデバイス名として使用されます
  - 例：`rainbow2.h` → `RAINBOW2`
  - 例：`dao_fps.h` → `DAO_FPS`

## 新しいデバイスの追加方法

1. `devices/` フォルダに新しいヘッダファイルを追加（例：`my_device.h`）
2. ビルド時に `-DIIDX_DEVICE=MYDEVICE` を指定してコンパイル
   ```bash
   cmake -B build -DIIDX_DEVICE=MYDEVICE
   make -C build iidxcon_pipico
   ```

## 既存のデバイス

- `dao_fps.h` - DAO FPS 対応デバイス
- `iidx_ps2.h` - PS2 対応デバイス（デフォルト）
- `rainbow2.h` - Rainbow 2 対応デバイス
- `rainbow2plus.h` - Rainbow 2 Plus 対応デバイス

## 注意事項

- ヘッダファイル名とデバイス名の対応は自動的に生成されます
- 大文字小文字は区別されません（`Rainbow2.h` と `rainbow2.h` は同じ `RAINBOW2` として扱われます）
- 重複するデバイス名（例：`my_device.h` と `MY_DEVICE.h`）は避けてください
