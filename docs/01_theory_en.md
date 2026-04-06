# 🎲 Physics of 3-Axis Attitude Control

> Why can a cube balance on its corner? A theoretical explanation with equations

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Academic Disciplines Used in This Project](#2-academic-disciplines-used-in-this-project)
3. [3D Inverted Pendulum Model](#3-3d-inverted-pendulum-model)
4. [Reaction Wheel Principle](#4-reaction-wheel-principle)
5. [Deriving the Equations of Motion](#5-deriving-the-equations-of-motion)
6. [Coordinate System and Cube Geometry](#6-coordinate-system-and-cube-geometry)
7. [PID Control Theory](#7-pid-control-theory)
8. [Motor Mixing](#8-motor-mixing)
9. [Attitude Estimation Algorithm](#9-attitude-estimation-algorithm)
10. [Stability Condition Analysis](#10-stability-condition-analysis)
11. [Summary](#11-summary)

---

## 1. Introduction

Balancing a cube on its corner — intuitively, it seems impossible. However, by combining **reaction wheels** with **feedback control**, this unstable equilibrium can be maintained.

This document explains the underlying physics using equations.

### 1.1 The Physical Essence of This Project

A cube balanced on its corner is essentially a **3D Inverted Pendulum**.

- 🔴 **Unstable equilibrium**: When the center of gravity is **directly above** the pivot point (corner), the system is in mechanical equilibrium, but any tiny disturbance will cause it to topple
- 🟢 **Active stabilization**: Three reaction wheels generate torque through **exchange of angular momentum**, correcting any tilt
- 🔵 **Feedback control**: An IMU sensor detects tilt, and PID control computes the appropriate motor torque

### 1.2 Understanding Through Everyday Examples

This principle is actually used in many familiar situations:

| Everyday Example | Shared Principle |
|---------|-------------|
| **Riding a bicycle** | Feel a tilt → turn the handlebars to correct → feedback control |
| **Figure skating spin** | Pull arms in → spin faster → conservation of angular momentum |
| **Satellite attitude control** | Change orientation using reaction wheels → same principle as this project! |
| **Segway** | Balances on 2 wheels → single-axis inverted pendulum |

> 💡 The fun of this project is realizing satellite-grade technology at palm size.
> NASA spacecraft also use reaction wheels for attitude control.

### 1.3 Why Is the "Corner" Special?

A cube's normal stable resting position is on a **face**. Balancing on a corner is physically possible, but
**statically unstable**.

The types of equilibrium can be organized as follows:

```
安定平衡（面の上）     不安定平衡（角の上）     中立平衡（球）
     ___                    ●                      ○
    /   \                  / \                 ─────────
   /  ●  \               /   \
  / 谷底  \             / 山頂 \              どこでも平衡
```

- **Stable equilibrium**: A ball at the bottom of a valley. Push it slightly and it rolls back
- **Unstable equilibrium**: A ball on a hilltop. Push it slightly and it rolls away ← **This is our case!**
- **Neutral equilibrium**: A ball on a flat surface. Push it and it stays wherever it stops

A cube balanced on its corner is in the same unstable equilibrium as a "ball on a hilltop."
**It will topple unless actively balanced at all times.**

---

## 2. Academic Disciplines Used in This Project

This project cannot be completed with a single discipline alone.
It is a textbook example of **mechatronics (robotics)**, combining multiple fields of study.

### 2.1 Discipline Map

```
┌─────────────────────────────────────────────────────────────┐
│                  3軸姿勢制御モジュール                       │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │ 物理学   │  │ 数学     │  │制御工学  │  │電気電子  │   │
│  │          │  │          │  │          │  │工学      │   │
│  │・古典力学│  │・線形代数│  │・PID制御 │  │・回路設計│   │
│  │・剛体力学│  │・微分方程│  │・状態空間│  │・モーター│   │
│  │・角運動量│  │  式      │  │  表現    │  │  制御    │   │
│  │  保存則  │  │・三角関数│  │・ディジタ│  │・センサ  │   │
│  │          │  │・テイラー│  │  ル信号  │  │  工学    │   │
│  │          │  │  展開    │  │  処理    │  │・PWM     │   │
│  └──────────┘  └──────────┘  └──────────┘  └──────────┘   │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐                  │
│  │ 情報工学 │  │ 機械工学 │  │通信工学  │                  │
│  │          │  │          │  │          │                  │
│  │・組込み  │  │・3D CAD  │  │・Bluetooth│                 │
│  │  プログラ│  │・3Dプリン│  │・シリアル│                  │
│  │  ミング  │  │  ト      │  │  通信    │                  │
│  │・I2C通信 │  │・機構設計│  │          │                  │
│  │・割り込み│  │・重心設計│  │          │                  │
│  └──────────┘  └──────────┘  └──────────┘                  │
└─────────────────────────────────────────────────────────────┘
```

### 2.2 Details of Each Discipline and How It's Used in This Project

#### 🔬 Physics (Classical Mechanics / Rigid Body Dynamics)

The most fundamental discipline. It directly answers "why can it balance on a corner?"

| Physical Law | How It's Used in This Project | Chapter |
|---------|----------------------|-----------|
| **Newton's laws of motion** | Rotational form ($\tau = I\alpha$) to derive equations of motion | Chapter 5 |
| **Conservation of angular momentum** | Accelerate a wheel → generates reaction torque on the body | Chapter 4 |
| **Gravitational torque** | $mgl\sin\theta$ — the force trying to topple it | Chapter 5 |
| **Moment of inertia** | $I = \frac{1}{6}ma^2$ — resistance to rotational acceleration | Chapter 3 |

> 📚 **To learn more**: University-level physics (mechanics), analytical mechanics textbooks

#### 📐 Mathematics

The tools for describing physical phenomena quantitatively.

| Mathematical Tool | What It's Used For | Specific Example |
|-----------|-----------|--------|
| **Differential equations** | Describing and analyzing equations of motion | $I\ddot{\theta} = mgl\theta - \tau$ |
| **Linear algebra** | Mixing matrix, state-space representation | 3×2 motor distribution matrix |
| **Trigonometry** | Geometry of motor placement | $\cos 30° = 0.866$ |
| **Taylor expansion** | Small-angle approximation | $\sin\theta \approx \theta$ |
| **Exponential functions** | Describing divergence of unstable systems | $\theta(t) = \theta\_0 e^{\lambda t}$ |
| **atan2 function** | Computing angle from acceleration | $\theta = \text{atan2}(a\_y, a\_z)$ |

> 📚 **To learn more**: Calculus, linear algebra, engineering mathematics

#### 🤖 Control Engineering

The theory of stabilizing unstable systems. The **cornerstone** of this project.

| Control Theory | What It Does | In One Phrase |
|---------|-----------|-----------|
| **Feedback control** | Repeated cycle of sensor → compute → drive motor | "Measure → Think → Act" loop |
| **PID control** | Corrects error using proportional, integral, and derivative terms | The most widely used basic controller |
| **Stability analysis** | Mathematically determines whether a system will diverge | Positive eigenvalue → unstable |
| **Discretization** | Implementing continuous systems on digital (microcontroller) hardware | Sampling every 10 ms |
| **Complementary filter** | Combines the strengths of two different sensors | Gyro (fast) + Accelerometer (accurate) |

> 📚 **To learn more**: Introduction to control engineering, modern control theory, digital signal processing

#### ⚡ Electrical and Electronic Engineering

The hardware that physically realizes the theory.

| Technology | Role |
|------|------|
| **Circuit design** | Power supply circuit (7805 regulator), voltage divider (battery voltage monitoring) |
| **PWM control** | Controlling motor speed via pulse-width modulation |
| **I2C communication** | Digital communication protocol between microcontroller and IMU sensor |
| **ADC (Analog-to-Digital Conversion)** | Converting analog battery voltage to a digital value |
| **PCB design** | Designing circuit boards, generating Gerber data, ordering fabrication |
| **MEMS sensors** | MPU6050 — Micro-Electro-Mechanical System detecting acceleration and angular velocity |

> 📚 **To learn more**: Electric circuits, electronic circuits, sensor engineering

#### 💻 Computer Science (Embedded Programming)

Implementing the theory as software.

| Technology | Description |
|------|------|
| **Embedded C/C++** | Microcontroller programming using the Arduino framework |
| **Real-time processing** | Reliably reading sensors → computing → driving motors on a 10 ms cycle |
| **Interrupt handling** | Counting encoder rotation pulses without missing any |
| **Fixed-point arithmetic** | High-speed filter computation using bit shifts (`>> 4` = 1/16) |
| **Bluetooth SPP** | Wireless real-time tuning of PID parameters |

> 📚 **To learn more**: Microcontroller programming, real-time OS, embedded systems

#### 🔩 Mechanical Engineering

Designing and manufacturing the physical structure.

| Technology | Description |
|------|------|
| **3D CAD** | 3D design of the enclosure, wheels, and brackets |
| **3D printing** | Fabricating parts in PLA plastic (considering precision and strength) |
| **Center-of-gravity design** | Arranging components so the center of gravity is as centered as possible when corner-standing |
| **Reaction wheel design** | Adjusting moment of inertia (adding bolts around the rim for weight tuning) |
| **Fastener design** | Assembly structure using M3 bolts and nuts |

> 📚 **To learn more**: Mechanical design, 3D CAD, strength of materials

### 2.3 How the Disciplines Relate

```
        物理学（なぜ動くか）
            │
            ▼
        数学（定量的に記述）
            │
            ▼
      ┌─────┴─────┐
      ▼           ▼
  制御工学      電気電子工学
 （どう制御）   （回路で実現）
      │           │
      └─────┬─────┘
            ▼
        情報工学
     （プログラムで実装）
            │
            ▼
        機械工学
    （物理的な構造体）
            │
            ▼
      ★ 完成品 ★
  角で立つ立方体！
```

> 💡 **Mechatronics** = Mechanics + Electronics + Informatics —
> a coined term. This project is a perfect embodiment of it.

---

## 3. 3D Inverted Pendulum Model

### 2.1 System Definition

Consider a cube placed on one of its corners.

```
        ↑ z (鉛直上向き)
        |
        ● 重心 (Center of Mass)
        |  ← 距離 l
        |
       ╱╲ 支点 (角)
──────╱──╲────── 地面
```

- **Pivot point**: One vertex (corner) of the cube. The point of contact with the ground
- **Center of mass**: The geometric center of the cube
- **Distance l**: The distance from the pivot to the center of mass

### 2.2 Geometric Parameters

For a cube with edge length $a$:

**Distance from pivot to center of mass:**

$$l = \frac{\sqrt{3}}{2} a$$

The space diagonal of a cube is $\sqrt{3}a$, and the distance from a corner to the center is half of that.

**Moment of inertia of the cube (about the center of mass):**

$$I_{body} = \frac{1}{6} m a^2$$

where $m$ is the mass of the cube. (For a uniform cube, this value is the same about all principal axes.)

### 2.3 Free Body Diagram

Forces and torques acting on a cube balanced on its corner:

```
            τ_wheel (リアクションホイールトルク)
               ↻
        ┌──────────┐
        │    ●     │← 重心: mg (重力, 下向き)
        │  / | \   │
        │ /  |  \  │
        └/───|───\─┘
        ╱    |    ╲
       ╱     ↓     ╲
      ●──── mg·l·sinθ ────→ 復元力（不安定方向！）
    支点
      ↑ N (垂直抗力)
```

**The crucial difference from a regular pendulum:**
- Regular pendulum: gravitational torque **restores** the system to equilibrium (stable)
- Inverted pendulum: gravitational torque **drives** the system away from equilibrium (unstable)

---

## 3. Reaction Wheel Principle

### 3.1 Conservation of Angular Momentum

The operating principle of reaction wheels is based on the **conservation of angular momentum**.

> 📝 **What is angular momentum?**
> It is the "rotational momentum" of a spinning object.
> Just as mass × velocity = linear momentum (for translational motion),
> moment of inertia × angular velocity = angular momentum (for rotational motion).
> $L = I \cdot \omega$
>
> The conservation of angular momentum states: "Unless an external torque is applied, the total angular momentum of a system remains constant."
> This is the same law that explains why a figure skater spins faster when pulling their arms in.

In a closed system with no external torque:

$$\vec{L}_{total} = \vec{L}_{body} + \vec{L}_{wheel} = \text{const}$$

where:
- $\vec{L}\_{total}$: Total angular momentum of the system
- $\vec{L}\_{body}$: Angular momentum of the cube body
- $\vec{L}\_{wheel}$: Angular momentum of the reaction wheel

### 3.2 Torque Generation Mechanism

When a wheel is **accelerated**, conservation of angular momentum causes the body to **rotate in the opposite direction**.

$$\vec{L}_{body} + \vec{L}_{wheel} = \text{const}$$

Taking the time derivative:

$$\dot{\vec{L}}_{body} + \dot{\vec{L}}_{wheel} = 0$$

$$\vec{\tau}_{body} = -\vec{\tau}_{wheel}$$

That is:

$$I_{body} \cdot \dot{\omega}_{body} = -I_{wheel} \cdot \dot{\omega}_{wheel}$$

> **☝️ This is the most important equation!**
> By controlling the wheel's angular acceleration $\dot{\omega}\_{wheel}$,
> we can apply a torque $\tau\_{body}$ to the body.

### 3.3 Intuitive Understanding

| Wheel Action | Effect on Body |
|:---:|:---:|
| Wheel accelerates (↻ faster) | Body rotates in reverse (↺) |
| Wheel decelerates (↻ slower) | Body rotates in the same direction (↻) |
| Wheel at constant speed | No torque on the body |

> 💡 **Key point**: What generates torque is the **change in rotational speed (angular acceleration)** of the wheel, not the rotational speed itself.

> 📝 **Similarities with spacecraft:**
> The International Space Station (ISS) and the Hubble Space Telescope also use
> reaction wheels for attitude control in space. In space, this is an extremely efficient
> method since orientation can be changed without consuming propellant. This project's cube
> is essentially a terrestrial miniature satellite.

---

## 4. Deriving the Equations of Motion

### 4.1 Single-Axis Equation of Motion

First, let's derive the equation of motion for a single axis (e.g., the Roll axis).

Let $\theta$ be the tilt angle from the vertical:

$$I_{body} \ddot{\theta} = mgl \sin\theta - \tau_{wheel}$$

Each term on the right-hand side:
- $mgl\sin\theta$: Destabilizing gravitational torque (accelerates the tilt)
- $\tau\_{wheel} = I\_{wheel}\dot{\omega}\_{wheel}$: Control torque from the reaction wheel

> 📝 **Understanding this equation intuitively:**
> - Left side $I\_{body} \ddot{\theta}$: "How quickly the body starts to rotate"
> - Right-side 1st term $mgl\sin\theta$: "The force of gravity trying to topple it" — grows larger with greater tilt
> - Right-side 2nd term $\tau\_{wheel}$: "The control force generated by the wheel" — used to counteract gravity
>
> In other words, **if the wheel's control force > gravity's toppling force, it won't fall!**

### 4.2 Linearization via Small-Angle Approximation

Near the upright position ($\theta \approx 0$), we can approximate $\sin\theta \approx \theta$:

> 📝 **What is the small-angle approximation?**
> When $\theta$ is small (roughly below 10°), $\sin\theta$ is approximately equal to $\theta$.
> Example: $\sin(5°) = \sin(0.087\text{rad}) = 0.0872 \approx 0.087$
> This approximation turns the nonlinear equation of motion into a linear one, making analysis much simpler.

$$I_{body} \cdot \ddot{\theta} \approx mgl \cdot \theta - I_{wheel} \cdot \dot{\omega}_{wheel}$$

> **☝️ The linearized equation of motion.** The left side is the body's rotational acceleration; the 1st term on the right is the gravitational toppling force; the 2nd term is the control torque.

This is an **unstable linear system**. When $\tau\_{wheel} = 0$:

$$\ddot{\theta} = \frac{mgl}{I_{body}} \theta$$

The solution diverges exponentially:

$$\theta(t) = \theta_0 \cdot e^{\sqrt{mgl/I_{body}} \cdot t}$$

→ **Without control, it will inevitably topple** (mathematical proof of unstable equilibrium)

> 📝 **Why does it topple exponentially?**
> The equation above says "the greater the tilt $\theta$, the greater the angular acceleration $\ddot{\theta}$ that increases the tilt further."
> This is positive feedback (a vicious cycle).
> The larger $mgl/I\_{body}$ is (heavier and higher center of gravity), the faster it falls.

### 4.3 Required Torque for Maintaining Balance

To maintain the inverted position:

$$\tau_{wheel} \geq mgl\theta_{max}$$

where $\theta\_{max}$ is the maximum tilt angle the sensor can detect and the controller can correct in time.

**Numerical example (this project's parameters):**
- $m \approx 0.7$ kg (frame + motors + battery)
- $l \approx 0.118$ m (for $a = 0.136$ m: $l = \sqrt{3}/2 \cdot 0.136$)
- $g = 9.81$ m/s²
- $\theta\_{max} = 7°= 0.122$ rad (from the code's threshold)

$$\tau_{min} = 0.7 \times 9.81 \times 0.118 \times 0.122 \approx 0.099 \text{ N·m}$$

→ Each motor must be capable of producing at least **approximately 0.1 N·m** of torque.

### 4.4 State-Space Representation

With state vector $\vec{x} = [\theta, \dot{\theta}]^T$ and control input $u = \tau\_{wheel}$:

$$\dot{\vec{x}} = \begin{bmatrix} 0 & 1 \\ \frac{mgl}{I_{body}} & 0 \end{bmatrix} \vec{x} + \begin{bmatrix} 0 \\ -\frac{1}{I_{body}} \end{bmatrix} u$$

The eigenvalues are $\lambda = \pm\sqrt{mgl/I\_{body}}$. Since there is a positive real eigenvalue, the system is **unstable in open loop**.

---

## 5. Coordinate System and Cube Geometry

### 5.1 Body Frame Definition

The coordinate system when the cube is balanced on its corner:

```
           ↑ Yaw軸 (z)
           |     （鉛直軸）
           |
    ──── Roll軸 (x)
         ╱
        ╱
  Pitch軸 (y)
```

- **Roll (x-axis)**: Tilt in the fore–aft direction
- **Pitch (y-axis)**: Tilt in the left–right direction
- **Yaw (z-axis)**: Rotation about the vertical axis

### 5.2 Motor Placement Geometry

Three motors (CTR, RH, LH) are mounted on three faces of the cube:

```
        上から見た図（点倒立時）

           CTR (Motor1)
            │  Roll軸方向
            │
            ● 重心
           ╱ ╲
          ╱   ╲
    LH ──╱ 60° ╲── RH
  (Motor3)       (Motor2)
```

- **CTR (Center) motor**: Mounted **perpendicular** to the Roll axis
- **RH (Right Hand) motor**: Positioned at **30°** from the Roll axis
- **LH (Left Hand) motor**: Positioned at **−30°** from the Roll axis

### 5.3 Trigonometric Distribution

From the angle each motor's rotation axis makes with the Roll/Pitch axes:

| Motor | Roll Component | Pitch Component |
|---------|---------|-----------|
| CTR | $\cos(0°) = 1$ | $\cos(90°) = 0$ |
| RH | $\cos(60°) = 0.5$ | $\cos(30°) = 0.866$ |
| LH | $-\cos(60°) = -0.5$ | $\cos(30°) = 0.866$ |

---

## 6. PID Control Theory

### 6.1 Basic PID Control

A PID controller computes the control input $u(t)$ based on the error $e(t)$:

$$u(t) = K_p \cdot e(t) + K_i \int_0^t e(\tau) d\tau + K_d \frac{de(t)}{dt}$$

Role of each term:
- **P (Proportional) term**: Correction force proportional to the current error → response speed
- **I (Integral) term**: Corrects accumulated past errors → eliminates steady-state error
- **D (Derivative) term**: Damping force based on the rate of change of error → suppresses oscillation

> 📝 **PID explained with a driving analogy:**
>
> Imagine driving a car to a destination:
> - **P (Proportional)**: "Far from the destination → press the gas harder." The larger the deviation, the stronger the correction
> - **I (Integral)**: "I've been drifting slightly right for a while → keep steering slightly left." Corrects accumulated error
> - **D (Derivative)**: "A sharp curve is coming up → brake early." Predictive correction based on the rate of change
>
> By combining all three PID terms, you can reach the target "quickly," "accurately," and "without oscillation."

### 6.2 Angular Velocity PID (This Project's Implementation)

In this project, rather than directly PID-controlling the angle, we PID-control the **angular velocity**.

> 📝 **Why control angular velocity instead of angle?**
> If you PID-control the angle directly, the derivative term D becomes the "rate of change of angle" = angular velocity.
> Since the gyro sensor directly outputs angular velocity, using it as-is results in less noise
> and easier tuning. The imo Lab. design also adopts this approach.

**Step 1: Automatic target angle correction**

$$\theta_{target}(t+\Delta t) = \theta_{target}(t) + K_t \cdot \Delta t \cdot (\theta_{target}(t) - \theta(t))$$

> 📝 The assembled cube is not perfectly symmetric, so the sensor's 0° and the actual balance
> point are slightly offset. This formula automatically learns and corrects that offset. A larger $K\_t$ corrects faster.

**Step 2: Compute target angular velocity**

$$\omega_{target} = K_a \cdot (\theta_{target} - \theta)$$

The target angular velocity is derived from the angle error.

**Step 3: Angular velocity PID control**

$$P = \omega_{target} - \omega_{gyro}$$

$$I = \theta \quad \text{（角度そのものを積分項として使用）}$$

$$D = \omega_{gyro}(t) - \omega_{gyro}(t-1) \quad \text{（微分先行型）}$$

$$u = K_p \cdot P + K_i \cdot I - K_d \cdot D$$

> 📝 **What is derivative-on-measurement?**
> The standard D term uses the "rate of change of error," but here we use the "rate of change of gyro angular velocity."
> This technique prevents a sudden spike in output (derivative kick) when the setpoint changes abruptly.

### 6.3 Discretization (Digital Implementation)

The microcontroller implementation uses discrete time:

$$P_k = \omega_{target,k} - \omega_{gyro,k}$$

$$I_k = I_{k-1} + P_k \cdot \Delta t$$

$$D_k = \omega_{gyro,k} - \omega_{gyro,k-1}$$

$$u_k = K_p \cdot P_k + K_i \cdot I_k - K_d \cdot D_k$$

Control period: $\Delta t = 10$ ms (100 Hz) — from the code: `if (currentTime - loopTimer >= 10000)`.

### 6.4 Initial PID Parameter Values

Initial values extracted from the code:

| Parameter | Unit 1 (ESP32) | Unit 2 (Pico) | Role |
|-----------|:---:|:---:|------|
| $K\_t$ (Ks) | 1.0 | 1.3 | Target angle correction speed |
| $K\_a$ (Kp_angle) | 5.0 | 5.5 | Angle-to-angular-velocity conversion gain |
| $K\_p$ | 60.0 | 32.0 | Proportional gain |
| $K\_i$ | 12.0 | 45.0 | Integral gain |
| $K\_d$ | 90.0 | 95.0 | Derivative gain |

---

## 7. Motor Mixing

### 7.1 Mixing Matrix

The Roll/Pitch PID outputs are distributed to the three motors:

$$\begin{bmatrix} u_{CTR} \\ u_{RH} \\ u_{LH} \end{bmatrix} = \begin{bmatrix} 1 & 0 \\ 0.5 & 0.866 \\ -0.5 & 0.866 \end{bmatrix} \begin{bmatrix} PID_{Roll} \\ PID_{Pitch} \end{bmatrix}$$

Expanding:

$$u_{CTR} = PID_{Roll}$$

$$u_{RH} = 0.5 \cdot PID_{Roll} + 0.866 \cdot PID_{Pitch}$$

$$u_{LH} = -0.5 \cdot PID_{Roll} + 0.866 \cdot PID_{Pitch}$$

### 7.2 Geometric Derivation

Why $0.866$ and $0.5$?

The three motors are arranged at **120° intervals** when viewed from above in the corner-standing position:

```
            0° (CTR)
            │
     240°───●───120°
     (LH)       (RH)
```

Each motor's contribution relative to the Roll axis (0° direction):

$$\text{Roll component} = \cos(\alpha)$$
$$\text{Pitch component} = \sin(\alpha)$$

| Motor | Angle α | cos(α) | sin(α) |
|---------|:---:|:---:|:---:|
| CTR | 0° | 1.0 | 0.0 |
| RH | 120° → **Roll: cos60°, Pitch: cos30°** | 0.5 | 0.866 |
| LH | 240° → **Roll: -cos60°, Pitch: cos30°** | -0.5 | 0.866 |

### 7.3 Yaw Axis Control (Unit 2 Only)

Unit 2 adds Yaw (z-axis rotation) control:

$$u_{CTR} = PID_{Roll} + PID_{Yaw}$$

$$u_{RH} = 0.5 \cdot PID_{Roll} + 0.866 \cdot PID_{Pitch} - PID_{Yaw}$$

$$u_{LH} = -0.5 \cdot PID_{Roll} + 0.866 \cdot PID_{Pitch} + PID_{Yaw}$$

Yaw control is achieved by applying torque in the **same direction** to all three wheels.

---

## 8. Attitude Estimation Algorithm

### 8.1 Problem Statement

The MPU6050 has two types of sensors:

| Sensor | Measurement | Strength | Weakness |
|--------|--------|------|------|
| Accelerometer | Gravity direction → angle | No drift | Sensitive to vibration and acceleration |
| Gyroscope | Angular velocity → angle (via integration) | Fast response | Drifts over time |

> 📝 **Why can't we use just one sensor?**
>
> - **Accelerometer only**: You can determine tilt from the direction of gravity, but motor vibrations make the readings very noisy.
>   Additionally, centrifugal forces from the spinning wheels are indistinguishable from gravity.
>
> - **Gyroscope only**: Integrating angular velocity gives you the angle, but small errors accumulate continuously (drift).
>   After just 30 seconds, the indicated angle can be completely wrong.
>
> → **Combining both compensates for each sensor's weaknesses** — that's the idea behind the complementary filter.

### 8.2 Complementary Filter

This project uses a **complementary filter** to fuse both sensors:

$$\theta_{est} = \alpha \cdot (\theta_{prev} + \omega_{gyro} \cdot \Delta t) + (1 - \alpha) \cdot \theta_{acc}$$

- $\alpha$: Filter coefficient (0.9–0.99, biased toward the gyro)
- $\omega\_{gyro} \cdot \Delta t$: Angular change obtained by integrating the gyro's angular velocity
- $\theta\_{acc}$: Angle computed from the accelerometer

> 📝 **Understanding the complementary filter intuitively:**
>
> ```
> 短期的（高周波）→ ジャイロを信頼（振動に強い、ドリフト前なら正確）
> 長期的（低周波）→ 加速度を信頼（ドリフトしない、真の重力方向がわかる）
> ```
>
> If $\alpha = 0.94$, it means "trust the gyro 94% and correct with the accelerometer 6%."
> Over short timescales, the gyro dominates for fast response; over long timescales, the accelerometer corrects drift.

### 8.3 Implementation Algorithm (Code Analysis)

In the actual code, the complementary filter is implemented as **gravity vector estimation**:

**Step 1: Rotate the gravity vector using the gyro**

$$\vec{V}_g' = \vec{V}_g + \vec{\omega} \times \vec{V}_g \cdot \Delta t$$

Update the direction of the gravity vector using the gyro's angular velocity (first-order approximation of a rotation matrix).

> 📝 The cross product $\vec{\omega} \times \vec{V}\_g$ computes the direction in which the
> gravity vector $V\_g$ rotates due to the angular velocity vector $\omega$.

**Step 2: Correction using the accelerometer (low-pass filter)**

$$\vec{V}_g = \vec{V}_g' + \frac{1}{16}(\vec{a}_{LPF} - \vec{V}_g')$$

Drift is corrected using the accelerometer values. $1/16$ corresponds to the bit shift `>> 4`.

> 📝 $\frac{1}{16}$ corresponds to $(1 - \alpha)$ in the complementary filter.
> That is, $\alpha = 15/16 = 0.9375$ (approximately 94% gyro trust).
> Using bit shifts enables high-speed computation on a microcontroller.

**Step 3: Computing angles**

$$\theta_{Roll} = \text{atan2}(V_{g,y}, V_{g,z})$$

$$\theta_{Pitch} = -\text{atan2}(V_{g,x}, \sqrt{V_{g,y}^2 + V_{g,z}^2})$$

$$\theta_{Yaw} += -\omega_z \cdot \Delta t \quad \text{（ジャイロ積分のみ）}$$

> ⚠️ Since the Yaw angle relies solely on gyro integration, it drifts over extended periods.

---

## 9. Stability Condition Analysis

### 9.1 Controllability

Conditions for maintaining the inverted position:

1. **Sufficient torque**: $\tau\_{max} > mgl\theta\_{max}$
2. **Sufficient response speed**: The control period $\Delta t$ must be much shorter than the system's time constant
3. **Accurate attitude estimation**: IMU accuracy and complementary filter bandwidth

### 9.2 System Time Constant

From the eigenvalues of the linearized model, the toppling time constant is:

$$T = \frac{1}{\sqrt{mgl/I_{body}}}$$

Using this project's values ($m = 0.7$ kg, $l = 0.118$ m, $I\_{body} = ma^2/6$):

$$I_{body} = \frac{0.7 \times 0.136^2}{6} = 0.00216 \text{ kg·m}^2$$

$$T = \frac{1}{\sqrt{\frac{0.7 \times 9.81 \times 0.118}{0.00216}}} = \frac{1}{\sqrt{375}} \approx 0.052 \text{ s} \approx 52 \text{ ms}$$

→ The 10 ms control period (100 Hz) is well within the 52 ms time constant ✅

### 9.3 Why It Can Balance on a Corner — Summary

The reason it can balance on a corner, in one sentence:

> **Because the reaction torque from the angular acceleration of the reaction wheels exceeds the gravitational toppling torque, and sufficiently fast feedback control continuously corrects any deviation.**

Expressed mathematically:

$$\underbrace{I_{wheel}\dot{\omega}_{wheel}}_{\text{Control torque}} > \underbrace{mgl\sin\theta}_{\text{Toppling torque}} \quad \text{and} \quad \underbrace{\Delta t}_{\text{Control period}} \ll \underbrace{T}_{\text{System time constant}}$$

---

## 10. Summary

| Physical Element | Equation | Implementation in This Project |
|-----------|------|----------------------|
| Conservation of angular momentum | $L\_{body} + L\_{wheel} = \text{const}$ | 3 reaction wheels |
| Equation of motion | $I\ddot{\theta} = mgl\theta - \tau\_{wheel}$ | $\tau\_{wheel}$ controlled via PID |
| PID control | $u = K\_p P + K\_i I - K\_d D$ | Angular velocity PID (imo Lab. method) |
| Motor mixing | cos 30° / cos 60° distribution matrix | 3 motors in 120° arrangement |
| Attitude estimation | Complementary filter (gravity vector method) | MPU6050 (accelerometer + gyro) |
| Stability condition | $\tau\_{max} > mgl\theta$, $\Delta t \ll T$ | 100 Hz control, time constant 52 ms |

---

## References

1. imo Lab. "[How to Control the Attitude of a 3-Axis Attitude Control Module](https://garchiving.com/how-to-3axis-control-self-balancing-cube/)"
2. remrc. "[Self-Balancing-Cube](https://github.com/remrc/Self-Balancing-Cube)" GitHub
3. willem-pennings. "[balancing-cube](https://github.com/willem-pennings/balancing-cube)" GitHub
4. Classical mechanics textbooks — Euler's rotation equations, rigid body dynamics
