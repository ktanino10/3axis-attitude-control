# 💻 ソフトウェア環境セットアップガイド

> Arduino IDE と Processing IDE の環境構築手順

---

## 1. Arduino IDE セットアップ

### 1.1 インストール

1. [Arduino IDE 2.x](https://www.arduino.cc/en/software) をダウンロード・インストール

### 1.2 Raspberry Pi Pico W ボードの追加

1. Arduino IDE を起動
2. **File** → **Preferences** → **Additional boards manager URLs** に以下を追加:
   ```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```
3. **Tools** → **Board** → **Boards Manager** で `pico` を検索
4. **Raspberry Pi Pico/RP2040** (by Earle F. Philhower) をインストール
5. ボード選択: **Tools** → **Board** → **Raspberry Pi Pico W**

### 1.3 ESP32 ボードの追加（1号機を使う場合）

1. **Preferences** → **Additional boards manager URLs** に追加:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
2. **Boards Manager** で `esp32` を検索・インストール
3. ボード選択: **ESP32 Dev Module**

### 1.4 ライブラリ

標準の `Wire.h` のみ使用。追加ライブラリのインストールは不要。

> ⚠️ Pico W 版は `SerialBT.h` というカスタムライブラリを使用している。
> imo Lab. の頒布データに含まれるか、Pico W の内蔵 BT を使う場合は
> 別途ライブラリの調整が必要。

### 1.5 ファームウェアの書き込み

**Pico W の場合:**
1. Pico W の **BOOTSEL** ボタンを押しながら USB 接続
2. Arduino IDE でポートを選択
3. `firmware/pico_w/3axis_control.ino` を開く
4. **Upload** ボタンでコンパイル＆書き込み

**ESP32 の場合:**
1. USB で接続（ドライバが自動認識されない場合は CP2102/CH340 ドライバをインストール）
2. Arduino IDE でポートを選択
3. `firmware/esp32/3axis_control.ino` を開く
4. **Upload** ボタン

### 1.6 シリアルモニタで動作確認

1. **Tools** → **Serial Monitor** を開く
2. ボーレート: **115200**
3. 正常なら `dt:`, `roll:`, `pitch:` 等のデータが流れる

---

## 2. Processing IDE セットアップ

Processing はPIDパラメータのリアルタイムチューニングGUIとして使用する。

### 2.1 インストール

1. [Processing 4.x](https://processing.org/download) をダウンロード・インストール

### 2.2 ライブラリの追加

1. Processing を起動
2. **Sketch** → **Import Library** → **Manage Libraries**
3. 以下を検索・インストール:
   - **ControlP5** — GUI コントロール（スライダー、テキストフィールド等）

> `processing.serial.*` は標準で含まれているため追加不要。

### 2.3 チューニングGUIの使い方

1. `processing/tuning_gui.pde` を Processing で開く
2. **COM ポート番号を修正** — ファイル内の `Serial.list()[X]` や `"COM4"` を自分の環境に合わせる
3. **Run** ボタンで起動
4. キューブと Bluetooth 接続した状態で:
   - スライダーで Kp / Ki / Kd / Ka / Kt を調整
   - リアルタイムで角度・角速度グラフを確認
   - 安定するパラメータを探る

### 2.4 Bluetooth 接続

1. PC の Bluetooth 設定で ESP32/Pico W をペアリング
2. 仮想 COM ポートが割り当てられる（例: COM4, COM5）
3. Processing の Serial ポートをその COM ポートに設定

---

## 3. トラブルシューティング

| 症状 | 対処法 |
|------|--------|
| ボードが認識されない | USB ケーブルがデータ対応か確認（充電専用ケーブル不可） |
| コンパイルエラー | ボード選択が正しいか確認。Pico W 用コードに ESP32 ボードを選んでいないか |
| シリアルモニタに何も出ない | ボーレート 115200 か確認。リセットボタンを押す |
| Processing でポートが見つからない | Bluetooth ペアリング済みか確認。デバイスマネージャで COM ポート番号を確認 |
| IMU の値がおかしい | I2C 接続を確認（SDA/SCL の接続、プルアップ抵抗） |
