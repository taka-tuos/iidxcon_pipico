# デバイス設定ファイル

このフォルダには、各コントローラーデバイス固有の設定ヘッダファイルを配置します。

## ファイル名の制約

- **拡張子は `.h` のみ**を使用してください（例：`my_device.h`）
- **ファイル名は英数字とアンダースコアのみ**にしてください
  - 特殊文字、スペース、ハイフンなどは避けてください
- **ファイル名は自動的に大文字に変換**されてデバイス名として使用されます
  - 例：`rainbow2.h` → `RAINBOW2`
  - 例：`dao_fps.h` → `DAO_FPS`

## 新しいデバイスの追加方法

1. `devices/` フォルダに新しいヘッダファイルを追加（例：`my_device.h`）
2. CMake 実行時に `-DIIDX_DEVICE=MYDEVICE` を指定
   ```bash
   cmake -B build -DIIDX_DEVICE=MYDEVICE
   ```
3. ビルド
   ```bash
   cmake --build build
   ```

## 既存のデバイス

- **`dao_fps.h`** - DJ DAO 製の「FPS」コントローラー用
- **`iidx_ps2.h`** - PS2 向け IIDX コントローラー（コナミ純正）用 ※デフォルト
- **`rainbow2.h`** - Rainbow Controller 2 用
- **`rainbow2plus.h`** - Rainbow Controller 2 Plus 用

## 注意事項

- ヘッダファイル名からデバイス名は自動的に生成されます（ファイル名の大文字小文字は区別されません）
- 重複するデバイス名になるファイル名（例：`my_device.h` と `MY_DEVICE.h`）は避けてください
- 各デバイスファイルでは、GPIO ピン配置や PS/2 対応設定などを定義します
