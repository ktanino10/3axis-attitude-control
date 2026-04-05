# Bill of Materials (BOM) — Self-Balancing Cube

> Reference design: imo Lab. (garchiving.com)
> Created: 2026-04-05

> 🌐 **Note for international buyers:**
> AliExpress links ship worldwide. Amazon.co.jp may offer international shipping depending on the seller.
> Akizuki Denshi (秋月電子) is a Japan-only retailer — international buyers should source equivalent parts from local suppliers or AliExpress.

---

## 📋 Version Comparison Summary

| Item | Unit 1 (ESP32 version) | Unit 2 (Pico version) |
|------|:---:|:---:|
| Microcontroller | ESP32 DevKit V1 | Raspberry Pi Pico W |
| IMU | MPU6050 | MPU6050 |
| Current/Voltage sensor | INA219 ✅ | None (voltage only via ADC) |
| Motor driver | PWM/DIR/BRAKE type ×3 | PWM/DIR/SS type ×3 |
| Motors | Brushed DC ×3 | Brushed DC ×3 |
| Encoder | Available (unused) | Available (1 axis used) |
| Bluetooth | ESP32 built-in SPP | External BT module required |
| Yaw axis control | None (2-axis: Roll/Pitch) | Yes (3-axis: Roll/Pitch/Yaw) |
| Enclosure size | □150mm | □136mm (compact) |
| PCB | Custom PCB | Custom PCB |
| **Recommended** | **Beginner-friendly (simple)** | **More complete (3-axis control)** |

### 💡 Recommendation: Build based on Unit 2 (Pico version)
- Full 3-axis attitude control (including Yaw)
- Compact & lightweight
- Raspberry Pi Pico W is widely available & affordable
- Encoder support enables motor RPM feedback

---

## 🔧 Complete Parts List (Based on Unit 2)

### 1. Microcontroller & Sensors

| # | Part Name | Model / Spec | Qty | Ref. Price | Where to Buy |
|---|-----------|--------------|:---:|----------:|--------------|
| 1 | Microcontroller board | **Raspberry Pi Pico W** | 1 | ¥1,240 | [Akizuki Denshi](https://akizukidenshi.com/catalog/g/g117947/) (cheapest, Japan only), [Amazon](https://www.amazon.co.jp/dp/B0BB69CH2T) |
| 2 | 6-axis IMU sensor | **MPU6050 module** (GY-521) | 1 | ¥220~ | [Amazon 5-pack](https://www.amazon.co.jp/dp/B07MPBMRWD), [Yahoo!](https://store.shopping.yahoo.co.jp/officek-diga/3axes-gyrosensor.html) |
| 3 | Current/Voltage sensor | **INA219 module** (optional) | 1 | ¥400 | Search "INA219" on Amazon |

> ⚠️ INA219 is not used in the Unit 2 code, but is useful for battery monitoring. Unit 1 code does use it.

### 2. Motors (with Built-in Driver & Encoder)

> 🔑 **Key finding from the schematic**:
> The motors are **integrated modules with a built-in driver circuit and encoder**.
> There is no need to purchase separate motor driver ICs!

Each motor module has an **8-pin connector** with the following signals:

| Pin | Signal Name | Description |
|:---:|-------------|-------------|
| 1 | 12V | Motor power (connected directly to battery) |
| 2 | GND | Ground |
| 3 | START/STOP | Run/stop control (labeled SS in code) |
| 4 | PWM | Speed control (8-bit, ~8 kHz) |
| 5 | DIR | Rotation direction |
| 6 | 5V | Logic power for encoder |
| 7 | SIGNAL_A | Encoder phase A |
| 8 | SIGNAL_B | Encoder phase B |

| # | Part Name | Model / Spec | Qty | Ref. Price | Where to Buy |
|---|-----------|--------------|:---:|----------:|--------------|
| 4 | Motor module | **DC motor with built-in driver (encoder, 8-pin)** | 3 | ¥800–2,000/ea | AliExpress, Amazon, Taobao |
| 5 | Shaft hub | **Metal shaft hub for 6 mm shaft** (set-screw type) | 3 | ¥200–500/ea | Amazon, Monotaro |

> 💡 **Shaft hub**: Couples the motor shaft (6 mm) to the reaction wheel.
> Screws into the 3D-printed wheel (wheel_x3.stl). Metal hubs recommended for strength & precision.

#### Motor Selection Details
**Requirements (confirmed from schematic & code):**
- **8-pin interface**: 12V / GND / START-STOP / PWM / DIR / 5V / ENC-A / ENC-B
- **Operating voltage**: 12V (3S LiPo = 11.1–12.6V)
- **PWM control**: 8-bit resolution (0–255), ~8 kHz
- **Built-in driver**: Integrated H-bridge drive circuit
- **Built-in encoder**: 2-phase (A/B) quadrature encoder

**Candidate Motors:**

#### ✅ Top Candidate: **Nidec 24H055M020** (BLDC servo motor with built-in driver)

imo Lab.'s blog mentions testing a motor with a "Nidec" logo,
and the schematic's 8-pin interface plus the code's control method (inverted PWM: `255 - abs(val)`)
are a perfect match for the Nidec 24H series specifications.

| Specification | Value |
|---------------|-------|
| Model | **Nidec 24H055M020** (or Nidec 24H series) |
| Type | Brushless DC servo motor (built-in driver) |
| Operating voltage | DC 12V (10–13V) |
| Max RPM | ~3,900 RPM |
| Dimensions | Φ42 mm × 34 mm |
| Shaft diameter | **6 mm** → select 6 mm shaft hub |
| Weight | ~118 g |
| Encoder | 100 lines (quadrature) |
| Control | PWM (duty cycle for speed, inverted operation) |
| Electronic brake | Yes |
| Forward/Reverse | DIR signal toggle |

**Purchase Links:**

| Retailer | Price Range | URL | Notes |
|----------|-------------|-----|-------|
| **AliExpress** | $5–10 (¥750–1,500) | [Nidec 24H 12V](https://www.aliexpress.com/item/1005006036612599.html) | Often used/salvaged parts, cheapest |
| **AliExpress** | $5–10 | [Nidec 24H 9–24V](https://www.aliexpress.com/item/1005005779568014.html) | Wider voltage range version |
| **Amazon.co.jp** | ¥1,500–3,000 | [Nidec 24H 12V](https://www.amazon.co.jp/dp/B0CG57F9DD) | Ships from Japan, slightly pricier |
| **Amazon.com** | $8–15 | [Nidec 24H](https://www.amazon.com/dp/B0DM1WVXW3) | US version |

> ⚠️ **Important Notes:**
> - Many listings are used/salvaged parts. Avoid shafts with helical gears — choose a flat (D-cut) shaft.
> - Wire colors may differ from the product listing (also noted on imo Lab.'s blog).
> - **Never reverse the power supply polarity!** It will damage the built-in driver.
> - Buy 3 (one per axis). Recommend buying 4 including a spare.

**Alternative Motors (if Nidec 24H is unavailable):**
1. Generic DC gearmotor + external motor driver (e.g. DRV8876) can be substituted
   → Code modifications (inverted PWM logic, etc.) will be needed.
2. Similar 8-pin interface motor with built-in driver (search AliExpress for equivalents).

> ⚠️ **Important**: Verify the motor mount hole diameter and shaft diameter against the 3D model.
> STL file: `2号機_Pico版/3Dモデル/BRKT_motor1_x3.stl`
> STEP file: `2号機_Pico版/3Dモデル/Assy.step` — open in CAD to check dimensions.

### 3. Power Supply

> 🔑 **From the schematic**: A 7805 regulator steps down the battery voltage (11.1V) to 5V.
> A voltage divider (R1 = 1.5 kΩ, R2 = 470 Ω) monitors battery voltage via ADC.

| # | Part Name | Model / Spec | Qty | Ref. Price | Where to Buy |
|---|-----------|--------------|:---:|----------:|--------------|
| 6 | Battery | **LiPo 3S (11.1V) 500–800 mAh** compact | 1 | ¥2,000–3,500 | [Amazon "3S 11.1V 500mAh"](https://www.amazon.co.jp/s?k=3S+11.1V+500mAh+Lipo) |
| 7 | LiPo charger | **Balance charger** (3S compatible) | 1 | ¥3,000–5,000 | [Amazon "Lipo charger 3S"](https://www.amazon.co.jp/s?k=Lipo+%E5%85%85%E9%9B%BB%E5%99%A8+3S) |
| 8 | 5V regulator | **NJM7805FA** (TO-220) | 1 | ¥60 | [Akizuki Denshi](https://akizukidenshi.com/catalog/g/g108678/) |
| 9 | Input capacitor | **0.1 µF ceramic** (C1) | 1 | ¥10 | Akizuki Denshi |
| 10 | Output capacitor | **33 µF electrolytic** (C2) | 1 | ¥20 | Akizuki Denshi |
| 11 | Voltage divider R1 | **1.5 kΩ** (battery voltage monitor) | 1 | ¥10 | Akizuki Denshi |
| 12 | Voltage divider R2 | **470 Ω** (battery voltage monitor) | 1 | ¥10 | Akizuki Denshi |
| 13 | Power switch | **Slide switch** or toggle switch | 1 | ¥100 | Akizuki Denshi |
| 14 | Connector | **XT30 / JST-XH** battery connector | 1 set | ¥200 | Amazon |

> 💡 **Battery Voltage Monitor**:
> Divider ratio = 470 / (1500 + 470) = 0.238
> Full charge 12.6V → ADC input 3.0V (within Pico ADC 3.3V range ✅)

### 4. PCB & Connectors

| # | Part Name | Model / Spec | Qty | Ref. Price | Where to Buy |
|---|-----------|--------------|:---:|----------:|--------------|
| 11 | Custom PCB | **PCB order** (using Gerber data) | 5+ pcs | $2–5 | [JLCPCB](https://jlcpcb.com/), [PCBGOGO](https://www.pcbgogo.jp/) |
| 12 | Pin headers | **2.54 mm pin header** (male) | 2–3 strips | ¥100 | Akizuki Denshi |
| 13 | Pin sockets | **2.54 mm pin socket** (female) | 2–3 strips | ¥100 | Akizuki Denshi |
| 14 | Hook-up wire | **AWG24–26 silicone wire** assorted colors | as needed | ¥500 | Amazon |
| 15 | Connectors | **JST-PH / JST-XH** for motor wiring | as needed | ¥300 | Akizuki Denshi |

### 5. Mechanical Parts (Screws & Spacers)

| # | Part Name | Spec | Qty | Ref. Price | Notes |
|---|-----------|------|:---:|----------:|-------|
| 16 | M3 bolts | Assorted lengths (low-profile head recommended) | 1 set | ¥500–800 | Motor mount, frame assembly |
| 17 | M3 nuts | Standard nuts | as needed | ¥200 | Frame fastening |
| 18 | M3 spacers | For PCB mounting (F-F or M-F) | 4–8 pcs | ¥300 | PCB bracket |
| 19 | M2 bolts | For sensor mounting | as needed | ¥200 | MPU6050 mounting |
| 20 | M3 flat-head screws | For reaction wheel perimeter | 30–40 pcs | ¥300 | Weight balance adjustment |
| 21 | Cable ties | For battery mounting | several | ¥100 | — |

### 6. Bluetooth Communication (for Unit 2)

| # | Part Name | Model / Spec | Qty | Ref. Price | Notes |
|---|-----------|--------------|:---:|----------:|-------|
| 22 | Bluetooth module | **HC-05** or **HC-06** SPP module | 1 | ¥500–800 | Check if Pico W's built-in BT is compatible |

> ⚠️ Pico W has built-in Bluetooth, but the code uses `SerialBT.h` (a custom library).
> Whether the Pico W's built-in BT works directly or an external HC-05 is needed — verify against the schematic.

---

## 📌 Complete Wiring Table (Unit 2 Pico Version — Confirmed from Schematic)

### Raspberry Pi Pico W Pin Assignment

```
                ┌─────────────┐
       GP0  ────┤ 1        40 ├──── VBUS
       GP1  ────┤ 2        39 ├──── VSYS ← +5V
      GND0  ────┤ 3        38 ├──── GND
  CTR PWM → GP2 ┤ 4        37 ├──── 3V3_EN
  CTR DIR → GP3 ┤ 5        36 ├──── 3V3_OUT
   RH PWM → GP4 ┤ 6        35 ├──── ADC_REF
   RH DIR → GP5 ┤ 7        34 ├──── GP28
      GND1  ────┤ 8        33 ├──── GND
   LH PWM → GP6 ┤ 9        32 ├──── GP27
   LH DIR → GP7 ┤10        31 ├──── GP26 ← BATT ADC
       GP8  ────┤11        30 ├──── RUN
       GP9  ────┤12        29 ├──── GP22
      GND2  ────┤13        28 ├──── GND
 CTR ENC A → GP10┤14       27 ├──── GP21
 CTR ENC B → GP11┤15       26 ├──── GP20 ← LH SS
  RH ENC A → GP12┤16       25 ├──── GP19 ← RH SS
  RH ENC B → GP13┤17       24 ├──── GP18 ← CTR SS
      GND3  ────┤18        23 ├──── GND
 LH ENC A → GP14┤19       22 ├──── GP17 ← I2C SCL
 LH ENC B → GP15┤20       21 ├──── GP16 ← I2C SDA
                └─────────────┘
```

### Motor Connector Wiring (8-pin × 3 motors)

| Pin | Motor CTR | Motor RH | Motor LH |
|:---:|-----------|----------|----------|
| 1 (12V) | +BATT | +BATT | +BATT |
| 2 (GND) | GND | GND | GND |
| 3 (SS) | GP18 | GP19 | GP20 |
| 4 (PWM) | GP2 | GP4 | GP6 |
| 5 (DIR) | GP3 | GP5 | GP7 |
| 6 (5V) | +5V | +5V | +5V |
| 7 (ENC-A) | GP10 | GP12 | GP14 |
| 8 (ENC-B) | GP11 | GP13 | GP15 |

### MPU6050 Connection

| MPU6050 Pin | Connected To |
|-------------|--------------|
| VCC | +5V |
| GND | GND |
| SCL | GP17 |
| SDA | GP16 |
| XDA | Not connected |
| XCL | Not connected |
| ADO | Not connected |
| INT | Not connected |

### Battery Voltage Monitor

```
+BATT ──┬── R1 (1.5kΩ) ──┬── R2 (470Ω) ──── GND
         │                 │
         │                 └── GP26 (ADC input)
         │
         └── 7805 VI ── 7805 VO ── +5V
             C1=0.1µF      C2=33µF
```

### 7. 3D-Printed Parts (already printed ✅)

#### Unit 2 Parts List
| Filename | Qty | Purpose |
|----------|:---:|---------|
| BRKT_motor1_x3.stl | 3 | Motor bracket (base) |
| BRKT_motor2_x12.stl | 12 | Motor bracket (legs) |
| BRKT_MPU6050_x1.stl | 1 | IMU sensor bracket |
| BRKT_PCBbord_x1.stl | 1 | PCB bracket |
| CORNER_CUBEx8.stl | 8 | Cube-shaped corner spacers |
| FRAME_bord_x3.stl | 3 | Frame (PCB/sensor side) |
| FRAME_motor_x3.stl | 3 | Frame (motor side) |
| wheel_x3.stl | 3 | Reaction wheels |

---

## 💰 Estimated Cost

| Category | Estimate |
|----------|--------:|
| Microcontroller (Pico W) + Sensors | ¥1,900 |
| Motor modules (built-in driver) ×3 | ¥2,400–6,000 |
| Battery & charger | ¥5,000–8,500 |
| Power circuit parts (7805, caps, resistors) | ¥100 |
| PCB (board house order) | ¥500–1,000 |
| Connectors & wire | ¥1,000 |
| Screws & spacers | ¥1,500 |
| Bluetooth module | ¥0–800 |
| **Total (estimate)** | **¥12,400 – ¥19,800** |

> ※ Excludes 3D-printed parts (already printed ✅), tools, soldering iron, etc.
> ※ The biggest cost drivers are the motors and the battery.

---

## 🔍 Items to Verify (Next Steps)

### Highest Priority
1. **✅ Schematic analysis complete** — Motors confirmed as "integrated 8-pin modules with built-in driver & encoder"
2. **Source the motors** — Find the same type of 8-pin motor module
   - Search AliExpress/Taobao for "DC motor built-in driver encoder 8pin 12V"
   - Look for motors similar to the Nidec motor mentioned on imo Lab.'s blog
   - Or purchase the ¥800 recipe from [BOOTH](https://booth.pm/ja/items/5992975) for the exact part numbers
3. **Check motor dimensions from 3D model** — Verify bracket mounting hole diameter and pitch
   - Open `D:\3軸姿勢制御\2号機_Pico版\3Dモデル\Assy.step` in CAD to check
4. **Battery size** — Confirm it fits inside the PCB bracket

### Consider Purchasing imo Lab.'s Recipe
- Available on [BOOTH](https://booth.pm/ja/items/5992975) for ¥800 — complete design dataset
- Likely includes the exact BOM and assembly instructions

### Nidec 24H Motor Reference Resources
- [Arduino Forum: Nidec 24H055M020 reverse-engineering](https://forum.arduino.cc/t/nidec-24h055m020-operation-solved-mostly/1429057) — Pinout analysis
- [GitHub: Nidec 24H control library](https://github.com/gcharles81/Nidec_24H_motor_control) — Arduino code example
- [YouTube: Nidec 24H wiring tutorial](https://www.youtube.com/watch?v=ZOyGFHkPrV4) — Wiring walkthrough
- [Arduino Forum: Self Balancing + Nidec 24H](https://forum.arduino.cc/t/self-balancing-reaction-wheel-problems-with-nidec-24h-055m020/1139918) — Practical example

---

## 🛒 Shopping Checklist (with purchase links)

> A copy-paste-ready checklist for purchasing. Listed in priority order.

### Top Priority (long lead time)

- [ ] **Motor: Nidec 24H055M020 ×4** (3 used + 1 spare)
  - 🏪 [Amazon.co.jp RUOHARD](https://www.amazon.co.jp/dp/B0FV3JQVN5) — ~¥1,500/ea, ships in a few days
  - 🏪 [Amazon.co.jp MESKE](https://www.amazon.co.jp/dp/B0D28GHSK1) — ~¥1,500/ea
  - 🏪 [AliExpress (cheapest)](https://ja.aliexpress.com/item/1005005567692278.html) — ~¥750/ea, delivery 2–3 weeks
  - ⚠️ **Choose a D-cut shaft. Avoid helical gear shafts.**

- [ ] **PCB order** (Gerber data: ZIP the contents of `hardware/gerber/` and upload)
  - 🏪 [JLCPCB](https://jlcpcb.com/) — $2–5 + shipping, delivery 1–2 weeks
  - 🏪 [PCBGOGO](https://www.pcbgogo.jp/) — Japanese-language support available

- [ ] **LiPo 3S battery 11.1V 500–800 mAh ×1**
  - 🏪 [Amazon "3S 11.1V 500mAh Lipo"](https://www.amazon.co.jp/s?k=3S+11.1V+500mAh+Lipo)
  - ⚠️ Choose a size that fits the PCB bracket

- [ ] **LiPo balance charger (3S compatible) ×1**
  - 🏪 [Amazon "Lipo charger 3S"](https://www.amazon.co.jp/s?k=Lipo+%E5%85%85%E9%9B%BB%E5%99%A8+3S)

### Akizuki Denshi — Bulk Order (Japan only — international buyers: source equivalents locally)

- [ ] **Raspberry Pi Pico W ×1** — [Akizuki ¥1,240](https://akizukidenshi.com/catalog/g/g117947/)
- [ ] **Voltage regulator NJM7805FA ×1** — [Akizuki ¥60](https://akizukidenshi.com/catalog/g/g108678/)
- [ ] **MLCC 0.1 µF ×1** — Akizuki ¥10
- [ ] **Electrolytic capacitor 33 µF 25V ×1** — Akizuki ¥20
- [ ] **Carbon resistor 1.5 kΩ ×1** — Akizuki ¥10
- [ ] **Carbon resistor 470 Ω ×1** — Akizuki ¥10
- [ ] **Pin header 1×40 ×2** — Akizuki ¥50/strip
- [ ] **Pin socket 1×40 ×2** — Akizuki ¥80/strip
- [ ] **Slide switch ×1** — Akizuki ¥50~

### Amazon — Bulk Order

- [ ] **MPU6050 (GY-521) module** — [Amazon 5-pack ¥1,100](https://www.amazon.co.jp/dp/B07MPBMRWD)
- [ ] **Shaft hub / coupling 6 mm ×3** — [Amazon "shaft coupling 6mm"](https://www.amazon.co.jp/s?k=%E3%82%B7%E3%83%A3%E3%83%95%E3%83%88%E3%82%AB%E3%83%83%E3%83%97%E3%83%AA%E3%83%B3%E3%82%B0+6mm)
- [ ] **M3 bolt & nut set** — Amazon "M3 bolt nut set" ¥500~
- [ ] **M2 bolt & nut set** — Amazon "M2 bolt nut set" ¥300~
- [ ] **M3 flat-head screws ×40** — Amazon ¥300~ (for wheel balance)
- [ ] **AWG24 silicone wire** — Amazon ¥500~
- [ ] **Cable ties** — Amazon or dollar store

### Optional

- [ ] **imo Lab. recipe** — [BOOTH ¥800](https://booth.pm/ja/items/5992975) (exact part numbers & assembly instructions)
