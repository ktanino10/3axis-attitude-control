# 🎲 3-Axis Attitude Control Module (Self-Balancing Cube)

> **[日本語版 README はこちら → README.md](README.md)**

A build log for a 3-axis attitude-control module using reaction wheels.
This inverted-pendulum robot — a cube that **balances on its corner (vertex)** — is being designed and implemented with the help of GitHub Copilot CLI.

## 🎯 Project Overview

<div align="center">

**A cube that stands on its corner?!**
Three reaction wheels control Roll / Pitch / Yaw across all three axes,
achieving unstable vertex balancing.

</div>

## 🚀 Getting Started — Recommended Reading Order

> **Note:** The documentation linked below is currently written in Japanese.
> English translations are being added progressively.

If you're new to the project, follow the documents in this order:

| Step | Document | Why |
|:----:|----------|-----|
| 1 | [Theory](docs/01_theory.md) ([🇬🇧 EN](docs/01_theory_en.md)) | Understand *why* it works — the physics and math behind vertex balancing. |
| 2 | [BOM](docs/02_bom.md) ([🇬🇧 EN](docs/02_bom_en.md)) | See *what to buy* — full parts list with sources. |
| 3 | [Circuit & Wiring](docs/03_circuit.md) | Learn *how to connect* — schematics and pin assignments. |
| 4 | [Software Setup](docs/04_software_setup.md) | Set up your *dev environment* — Arduino IDE / Processing IDE. |
| 5 | [Assembly](docs/05_assembly.md) | *Build it* — mechanical and electrical assembly guide. |
| 6 | [Tuning](docs/06_tuning.md) | *Make it balance* — PID parameter tuning procedure. |
| 7 | [Schedule](docs/schedule.md) | Project timeline and progress tracking. |

## 📖 Full Documentation Index

| # | Document | Contents |
|---|----------|----------|
| 1 | [Theory](docs/01_theory.md) ([🇬🇧 EN](docs/01_theory_en.md)) | Why can a cube stand on its corner? Physics explained with equations. |
| 2 | [BOM](docs/02_bom.md) ([🇬🇧 EN](docs/02_bom_en.md)) | Required parts and supplier list. |
| 3 | [Circuit & Wiring](docs/03_circuit.md) | Schematics, pin wiring tables. |
| 4 | [Software Setup](docs/04_software_setup.md) | Arduino IDE / Processing IDE setup. |
| 5 | [Assembly](docs/05_assembly.md) | Mechanical & electrical assembly guide. |
| 6 | [Tuning](docs/06_tuning.md) | PID parameter adjustment methods. |
| 📅 | [Schedule](docs/schedule.md) | Project timeline & progress management. |
| 📝 | [Experiment Log](experiments/log.md) | Daily experiment notes. |

## 🔧 Hardware Summary

| Component | Model / Spec |
|-----------|-------------|
| MCU | Raspberry Pi Pico W |
| IMU | MPU6050 (6-axis accelerometer + gyroscope) |
| Motors | Nidec 24H055M020 (driver-integrated BLDC) ×3 |
| Reaction Wheels | 3D-printed, Φ 116 mm ×3 |
| Battery | LiPo 3S 11.1 V |
| Frame | 3D-printed PLA, 136 mm cube |

## 📁 Repository Structure

```
├── docs/              # Documentation (theory, BOM, schematics, guides)
├── firmware/          # MCU firmware
│   ├── pico_w/       #   Raspberry Pi Pico W build
│   └── esp32/        #   ESP32 build
├── processing/        # PID tuning GUI (Processing)
├── hardware/          # Schematics & Gerber data
│   ├── gerber/
│   └── schematics/
└── experiments/       # Experiment logs
```

## 🙏 Acknowledgements & References

- [imo Lab.](https://garchiving.com/summary-of-self-balancing-cube/) — Reference design
- [remrc/Self-Balancing-Cube](https://github.com/remrc/Self-Balancing-Cube)
- [willem-pennings/balancing-cube](https://github.com/willem-pennings/balancing-cube)
- Development assisted by [GitHub Copilot CLI](https://github.com/features/copilot)

## 📄 License

MIT License
