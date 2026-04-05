# 🎲 3軸姿勢制御モジュール（Self-Balancing Cube）

リアクションホイールを用いた3軸姿勢制御モジュールの制作記録です。  
立方体が **角（頂点）で自立する** 倒立振子ロボットを、GitHub Copilot CLI を活用しながら設計・実装していきます。

## 🎯 プロジェクト概要

<div align="center">

**立方体が角で立つ？！**  
3つのリアクションホイールで Roll / Pitch / Yaw の3軸を制御し、  
不安定な頂点倒立を実現します。

</div>

## 📖 ドキュメント

| # | ドキュメント | 内容 |
|---|-------------|------|
| 1 | [理論解説](docs/01_theory.md) | なぜ角で立てるのか？数式による物理的説明 |
| 2 | [部品表（BOM）](docs/02_bom.md) | 必要な部品・購入先リスト |
| 3 | [回路図・配線](docs/03_circuit.md) | 回路図、ピン配線表 |
| 4 | [ソフトウェア環境](docs/04_software_setup.md) | Arduino IDE / Processing IDE セットアップ |
| 5 | [組立手順](docs/05_assembly.md) | 機械・電気の組立ガイド |
| 6 | [チューニング](docs/06_tuning.md) | PIDパラメータ調整方法 |
| 📅 | [実験スケジュール](docs/schedule.md) | 工程表・進捗管理 |
| 📝 | [実験ログ](experiments/log.md) | 日々の実験記録 |

## 🔧 ハードウェア構成

| コンポーネント | 型番・仕様 |
|--------------|-----------|
| マイコン | Raspberry Pi Pico W / ESP32 |
| IMU | MPU6050（6軸加速度・ジャイロ） |
| モーター | Nidec 24H055M020（ドライバ内蔵BLDC）×3 |
| リアクションホイール | 3Dプリント製 Φ116mm ×3 |
| バッテリー | LiPo 3S 11.1V |
| 筐体 | 3Dプリント製（PLA）□136mm |

## 📁 リポジトリ構成

```
├── docs/              # ドキュメント（理論・BOM・回路図・手順書）
├── firmware/          # マイコンファームウェア
│   ├── pico_w/       # Raspberry Pi Pico W 版
│   └── esp32/        # ESP32 版
├── processing/        # PIDチューニングGUI（Processing）
├── hardware/          # 回路図・ガーバーデータ
│   ├── gerber/
│   └── schematics/
└── experiments/       # 実験ログ
```

## 🙏 謝辞・参考

- [imo Lab.](https://garchiving.com/summary-of-self-balancing-cube/) — 設計の参考にさせていただきました
- [remrc/Self-Balancing-Cube](https://github.com/remrc/Self-Balancing-Cube)
- [willem-pennings/balancing-cube](https://github.com/willem-pennings/balancing-cube)
- 制作には [GitHub Copilot CLI](https://github.com/features/copilot) を活用しています

## 📄 ライセンス

MIT License
