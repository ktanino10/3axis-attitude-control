#include <Wire.h>
#include <hardware/pwm.h>
#include "SerialBT.h"

#define ROLL 0
#define PITCH 1
#define YAW 2

#define _Kt 1.3
#define _Ka 5.5
#define _Kp 32.0
#define _Ki 45.0
#define _Kd 95.0
#define _Kz 0.0
#define _rollAdj 0.0
#define _pitchAdj 0.0
#define _yawAdj 0.0

#define PIN_SDA (16u)
#define PIN_SCL (17u)
#define PIN_BATT_ADC (26u)

#define PIN_MOTOR1_PWM (2u)
#define PIN_MOTOR1_DIR (3u)
#define PIN_MOTOR1_SS (18u)
#define PIN_ENC1A (10u)
#define PIN_ENC1B (11u)

#define PIN_MOTOR2_PWM (4u)
#define PIN_MOTOR2_DIR (5u)
#define PIN_MOTOR2_SS (19u)
#define PIN_ENC2A (12u)
#define PIN_ENC2B (13u)

#define PIN_MOTOR3_PWM (6u)
#define PIN_MOTOR3_DIR (7u)
#define PIN_MOTOR3_SS (20u)
#define PIN_ENC3A (14u)
#define PIN_ENC3B (15u)

uint32_t currentTime, loopTimer, encTime, serialTime, BluetoothTime;
uint32_t dt, period;
volatile int16_t enc1_count;
int16_t motor1_speed;
int16_t motor1_PWM;
uint8_t vertical;

uint16_t batt;

float Kt, Ka, Kp, Ki, Kd, Kz;
float P[3], preP[2], I[2], D[2];
int16_t PID[3];
float targetAngle[2], targetOmega[3];
float rollAdj, pitchAdj, yawAdj;

uint16_t angCalibration;
int32_t accCal[3];
int32_t gyroCal[3];
typedef union {
  struct {
    int32_t X, Y, Z;
  } B32;
  struct {
    int16_t XL;
    int16_t X;
    int16_t YL;
    int16_t Y;
    int16_t ZL;
    int16_t Z;
  } B16;
} TYPE;

TYPE accRaw, accRawLPF;
TYPE gyroRaw, gyroRawLPF;
TYPE Vg = { 0, 0, 4096 * 65536 };
TYPE Vg0;
float angle[3];

void setup() {
  Serial.begin(115200);
  SerialBT.begin(115200);

  pinMode(PIN_MOTOR1_DIR, OUTPUT);
  pinMode(PIN_MOTOR1_SS, OUTPUT);
  digitalWrite(PIN_MOTOR1_DIR, HIGH);
  digitalWrite(PIN_MOTOR1_SS, LOW);

  pinMode(PIN_MOTOR2_DIR, OUTPUT);
  pinMode(PIN_MOTOR2_SS, OUTPUT);
  digitalWrite(PIN_MOTOR2_DIR, HIGH);
  digitalWrite(PIN_MOTOR2_SS, LOW);

  pinMode(PIN_MOTOR3_DIR, OUTPUT);
  pinMode(PIN_MOTOR3_SS, OUTPUT);
  digitalWrite(PIN_MOTOR3_DIR, HIGH);
  digitalWrite(PIN_MOTOR3_SS, LOW);

  gpio_set_function(PIN_MOTOR1_PWM, GPIO_FUNC_PWM);
  uint slice_num1 = pwm_gpio_to_slice_num(PIN_MOTOR1_PWM);
  motor1_controller(0);
  pwm_set_enabled(slice_num1, true);

  gpio_set_function(PIN_MOTOR2_PWM, GPIO_FUNC_PWM);
  uint slice_num2 = pwm_gpio_to_slice_num(PIN_MOTOR2_PWM);
  motor2_controller(0);
  pwm_set_enabled(slice_num2, true);

  gpio_set_function(PIN_MOTOR3_PWM, GPIO_FUNC_PWM);
  uint slice_num3 = pwm_gpio_to_slice_num(PIN_MOTOR3_PWM);
  motor3_controller(0);
  pwm_set_enabled(slice_num3, true);

  Wire.setSDA(PIN_SDA);
  Wire.setSCL(PIN_SCL);
  Wire.setClock(400000);
  Wire.begin();
  delay(500);

  setupMPU6050();
  readMPU6050();
  Vg.B16.X = accRaw.B16.X;
  Vg.B16.Y = accRaw.B16.Y;
  Vg.B16.Z = accRaw.B16.Z;
  accRawLPF.B16.X = accRaw.B16.X;
  accRawLPF.B16.Y = accRaw.B16.Y;
  accRawLPF.B16.Z = accRaw.B16.Z;

  attachInterrupt(PIN_ENC1A, CALL_ENC1A, CHANGE);
  attachInterrupt(PIN_ENC1B, CALL_ENC1B, CHANGE);
  Kt = _Kt;
  Ka = _Ka;
  Kp = _Kp;
  Ki = _Ki;
  Kd = _Kd;
  rollAdj = _rollAdj;
  pitchAdj = _pitchAdj;

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  currentTime = micros();
  loopTimer = currentTime;
  serialTime = currentTime;
  encTime = currentTime;
}

void loop() {
  dt = micros() - currentTime;
  while (1) {
    currentTime = micros();
    if (currentTime - loopTimer >= 10000) break;  //100Hz(10ms)
  }
  period = micros() - loopTimer;
  loopTimer = currentTime;

  batt = analogRead(PIN_BATT_ADC);
  computeAngle();
  angle[0] += rollAdj;
  angle[1] += pitchAdj;


  if (abs(angle[0]) < 2.0 && abs(angle[1]) < 2.0) vertical = 2;
  if (abs(angle[ROLL]) > 7.0 || abs(angle[PITCH]) > 7.00) vertical = 0;
  if (abs(angle[0]) < 5.0 && 30.0 < angle[1] && angle[1] < 40.0) vertical = 1;
  //if (abs(angle[0]) < 5.00) vertical = 1;
  //if (abs(angle[0]) > 10.0) vertical = 0;
  //if (abs(angle[0]) > 10.0 || angle[1] < 25 || 45 < angle[1]) vertical = 0;

  PID_controller();
  if (vertical == 1) {
    motor1_controller(PID[ROLL]);
    motor2_controller(0);
    motor3_controller(0);
  }
  if (vertical == 2) {
    motor1_controller(PID[ROLL] + PID[YAW]);
    motor2_controller(0.866 * PID[PITCH] + 0.5 * PID[ROLL] - PID[YAW]);
    motor3_controller(0.866 * PID[PITCH] - 0.5 * PID[ROLL] + PID[YAW]);
  }


  if (currentTime - encTime >= 100000) {
    encTime = micros();
    motor1_speed = enc1_count;
    enc1_count = 0;
  }

  if (currentTime - BluetoothTime >= 50000) {
    BluetoothTime = micros();
    BlueToothCom();
  }

  if (currentTime - serialTime >= 20000) {  //
    serialTime = micros();
    Serial.print("dt:");
    Serial.print((float)dt, 0);
    Serial.print(",");
    Serial.print("per.:");
    Serial.print((float)period, 0);
    Serial.print(",");
    Serial.print("roll:");
    Serial.print((float)angle[0], 3);
    Serial.print(",");
    Serial.print("Kz:");
    Serial.print((float)Kz, 1);
    Serial.print(",");
    Serial.print("yawTrim:");
    Serial.print((float)yawAdj, 1);
    Serial.print("\n");
  }
}

void PID_controller() {
  if (vertical == 1 || vertical == 2) {
    targetAngle[0] += period * 1e-6 * Kt * (targetAngle[0] - angle[0]);
    targetAngle[0] = constrain(targetAngle[0], -9, 9);
    targetAngle[1] += period * 1e-6 * Kt * (targetAngle[1] - angle[1]);
    targetAngle[1] = constrain(targetAngle[1], -9, 9);
    //roll
    targetOmega[0] = Ka * (targetAngle[0] - angle[0]);
    P[0] = targetOmega[0] - (gyroRawLPF.B16.X * 0.0152587890625);
    I[0] = angle[0];
    D[0] = (gyroRawLPF.B16.X * 0.0152587890625) - preP[0];
    preP[0] = (gyroRawLPF.B16.X * 0.0152587890625);
    PID[0] = Kp * P[0] + Ki * I[0] - Kd * D[0];
    //pitch
    targetOmega[1] = Ka * (targetAngle[1] - angle[1]);
    P[1] = targetOmega[1] - (gyroRawLPF.B16.Y * 0.0152587890625);
    I[1] = angle[1];
    D[1] = (gyroRawLPF.B16.Y * 0.0152587890625) - preP[1];
    preP[1] = (gyroRawLPF.B16.Y * 0.0152587890625);
    PID[1] = Kp * P[1] + Ki * I[1] - Kd * D[1];
    //yaw
    targetOmega[2] = yawAdj;
    P[2] = targetOmega[2] - gyroRawLPF.B16.Z * 0.0152587890625;
    PID[2] = Kz * P[2];

    PID[0] = constrain(PID[0], -255, 255);
    PID[1] = constrain(PID[1], -255, 255);
  }
  if (vertical == 0) {
    targetAngle[0] = 0;
    targetAngle[1] = 0;
    PID[0] = 1;
    PID[1] = 1;

    digitalWrite(PIN_MOTOR1_SS, LOW);
    digitalWrite(PIN_MOTOR2_SS, LOW);
    digitalWrite(PIN_MOTOR3_SS, LOW);
  }
}

// ****************************************************************
// * PWM出力
// ****************************************************************
void motor1_controller(int16_t val) {
  digitalWrite(PIN_MOTOR1_SS, HIGH);
  if (val > 0) digitalWrite(PIN_MOTOR1_DIR, 0);
  if (val < 0) digitalWrite(PIN_MOTOR1_DIR, 1);
  //digitalWrite(PIN_MOTOR1_SS, 1);
  //Duty 0-255 20kHz
  uint slice_num = pwm_gpio_to_slice_num(PIN_MOTOR1_PWM);
  pwm_set_wrap(slice_num, 255);
  pwm_set_clkdiv(slice_num, 64.941406);
  pwm_set_chan_level(slice_num, PWM_CHAN_A, 255 - abs(val));
}

void motor2_controller(int16_t val) {
  digitalWrite(PIN_MOTOR2_SS, HIGH);
  if (val > 0) digitalWrite(PIN_MOTOR2_DIR, 1);
  if (val < 0) digitalWrite(PIN_MOTOR2_DIR, 0);
  //digitalWrite(PIN_MOTOR1_SS, 1);
  //Duty 0-255 20kHz
  uint slice_num = pwm_gpio_to_slice_num(PIN_MOTOR2_PWM);
  pwm_set_wrap(slice_num, 255);
  pwm_set_clkdiv(slice_num, 64.941406);
  pwm_set_chan_level(slice_num, PWM_CHAN_A, 255 - abs(val));
}

void motor3_controller(int16_t val) {
  digitalWrite(PIN_MOTOR3_SS, HIGH);
  if (val > 0) digitalWrite(PIN_MOTOR3_DIR, 0);
  if (val < 0) digitalWrite(PIN_MOTOR3_DIR, 1);
  //digitalWrite(PIN_MOTOR1_SS, 1);
  //Duty 0-255 20kHz
  uint slice_num = pwm_gpio_to_slice_num(PIN_MOTOR3_PWM);
  pwm_set_wrap(slice_num, 255);
  pwm_set_clkdiv(slice_num, 64.941406);
  pwm_set_chan_level(slice_num, PWM_CHAN_A, 255 - abs(val));
}

// ****************************************************************
// * エンコーダー割り込み
// ****************************************************************
void CALL_ENC1A() {
  if (gpio_get(PIN_ENC1A) == 0) gpio_get(PIN_ENC1B) == 0 ? enc1_count++ : enc1_count--;
  if (gpio_get(PIN_ENC1A) == 1) gpio_get(PIN_ENC1B) == 1 ? enc1_count++ : enc1_count--;
}
void CALL_ENC1B() {
  if (gpio_get(PIN_ENC1B) == 0) gpio_get(PIN_ENC1A) == 1 ? enc1_count++ : enc1_count--;
  if (gpio_get(PIN_ENC1B) == 1) gpio_get(PIN_ENC1A) == 0 ? enc1_count++ : enc1_count--;
}

// ****************************************************************
// * 姿勢角度算出 * UNOR3:1060us ESP32:524us Pico(133MHz):498us Pico(200MHz):466us
// ****************************************************************
inline void computeAngle() {
  static uint32_t angTimer;
  uint32_t AttTime = micros();
  float dt2 = (AttTime - angTimer) * 0.000001;  //loop[s]
  float scale = 17.45329252 * dt2;              //(1/LSB)*PI*65536 = 17.45329252=>rad/s*65536
  angTimer = AttTime;

  readMPU6050();

  int32_t _sinX = gyroRaw.B16.X * scale;
  int32_t _sinY = gyroRaw.B16.Y * scale;
  int32_t _sinZ = gyroRaw.B16.Z * scale;

  Vg0.B32.X = Vg.B32.X + Vg.B16.Y * _sinZ - Vg.B16.Z * _sinY;
  Vg0.B32.Y = -Vg.B16.X * _sinZ + Vg.B32.Y + Vg.B16.Z * _sinX;
  Vg0.B32.Z = Vg.B16.X * _sinY - Vg.B16.Y * _sinX + Vg.B32.Z;

  Vg.B32.X = Vg0.B32.X;
  Vg.B32.Y = Vg0.B32.Y;
  Vg.B32.Z = Vg0.B32.Z;

  Vg.B32.X += (accRawLPF.B32.X - Vg.B32.X) >> 4;
  Vg.B32.Y += (accRawLPF.B32.Y - Vg.B32.Y) >> 4;
  Vg.B32.Z += (accRawLPF.B32.Z - Vg.B32.Z) >> 4;

  angle[ROLL] = _atan2(Vg.B16.Y, Vg.B16.Z) * 0.01;
  angle[PITCH] = -_atan2(Vg.B16.X, sqrt((int32_t)Vg.B16.Y * Vg.B16.Y + (int32_t)Vg.B16.Z * Vg.B16.Z)) * 0.01;
  angle[YAW] += -1 * gyroRaw.B16.Z * 0.0152587890625 * dt2;  //1/LSB => 1/65.536 deg/s
}

// ****************************************************************
// *** MPU6050 値読み取り ***
// ****************************************************************
inline void readMPU6050() {
  Wire.beginTransmission(0x68);  //MPU-6050 デバイスアドレス
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 14);

  while (Wire.available() < 14) {}
  accRaw.B16.X = Wire.read() << 8 | Wire.read();
  accRaw.B16.Y = Wire.read() << 8 | Wire.read();
  accRaw.B16.Z = Wire.read() << 8 | Wire.read();
  int16_t temperature = Wire.read() << 8 | Wire.read();
  gyroRaw.B16.X = Wire.read() << 8 | Wire.read();
  gyroRaw.B16.Y = Wire.read() << 8 | Wire.read();
  gyroRaw.B16.Z = Wire.read() << 8 | Wire.read();

  if (angCalibration == 2048) {
    accRaw.B16.X -= accCal[0];
    accRaw.B16.Y -= accCal[1];
    accRaw.B16.Z -= accCal[2];
    gyroRaw.B16.X -= gyroCal[0];
    gyroRaw.B16.Y -= gyroCal[1];
    gyroRaw.B16.Z -= gyroCal[2];
  }

  accRawLPF.B32.X += (accRaw.B32.X - accRawLPF.B32.X) >> 2;
  accRawLPF.B32.Y += (accRaw.B32.Y - accRawLPF.B32.Y) >> 2;
  accRawLPF.B32.Z += (accRaw.B32.Z - accRawLPF.B32.Z) >> 2;
  gyroRawLPF.B32.X += (gyroRaw.B32.X - gyroRawLPF.B32.X) >> 2;
  gyroRawLPF.B32.Y += (gyroRaw.B32.Y - gyroRawLPF.B32.Y) >> 2;
  gyroRawLPF.B32.Z += (gyroRaw.B32.Z - gyroRawLPF.B32.Z) >> 2;
}


// ****************************************************************
// *** MPU6050 初期設定 ***
// ****************************************************************
void setupMPU6050() {
  //内部クロック8MHz
  Wire.beginTransmission(0x68);  //MPU6050デバイスアドレス
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  //加速度センサー
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10);  //±8G LSB:4096
  Wire.endTransmission();
  //ジャイロ
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0x08);  //500deg/s LSB:65.536
  Wire.endTransmission();
  //DigitalLPF
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x05);  //≒43Hz 4.9ms遅れ
  Wire.endTransmission();

  for (angCalibration = 0; angCalibration < 2048; angCalibration++) {
    readMPU6050();
    delay(1);
    accCal[0] += accRaw.B16.X;
    accCal[1] += accRaw.B16.Y;
    accCal[2] += accRaw.B16.Z;
    gyroCal[0] += gyroRaw.B16.X;
    gyroCal[1] += gyroRaw.B16.Y;
    gyroCal[2] += gyroRaw.B16.Z;
  }
  accCal[0] /= 2048;
  accCal[1] /= 2048;
  accCal[2] = (accCal[2] / 2048) - 4096;
  gyroCal[0] /= 2048;
  gyroCal[1] /= 2048;
  gyroCal[2] /= 2048;

  //Manual Calibration
  //センサー水平
  //accCal[0] = 75;
  //accCal[1] = 88;
  //accCal[2] = -690;
  //gyroCal[0] = -52;
  //gyroCal[1] = 107;
  //gyroCal[2] = 47;

  //CUBE用（3軸用）
  //accCal[0] += 1672;
  //accCal[1] += -2896;
  //accCal[2] += 1731;

  //CUBE用（3軸用）
  accCal[0] = -1672 + accCal[0];
  accCal[1] = -2896 + accCal[1];
  accCal[2] = 2364 + 4096 + accCal[2];
}

// ********************************************************************************
// * atan2 近似式
// ********************************************************************************
inline int16_t _atan2(int16_t _y, int16_t _x) {
  int16_t x = abs(_x);
  int16_t y = abs(_y);
  float z;
  bool c;

  c = y < x;
  if (c) z = (float)y / x;
  else z = (float)x / y;

  int16_t a;
  //a = z * (-1556 * z + 6072);                     //2次曲線近似
  //a = z * (z * (-448 * z - 954) + 5894);          //3次曲線近似
  a = z * (z * (z * (829 * z - 2011) - 58) + 5741);  //4次曲線近似

  if (c) {
    if (_x > 0) {
      if (_y < 0) a *= -1;
    }
    if (_x < 0) {
      if (_y > 0) a = 18000 - a;
      if (_y < 0) a = a - 18000;
    }
  }

  if (!c) {
    if (_x > 0) {
      if (_y > 0) a = 9000 - a;
      if (_y < 0) a = a - 9000;
    }
    if (_x < 0) {
      if (_y > 0) a = a + 9000;
      if (_y < 0) a = -a - 9000;
    }
  }
  return a;
}

// ********************************************************************************
// * Bluetoothシリアル（SPP）
// ********************************************************************************
void BlueToothCom() {
  uint8_t inByteCnt = 19;
  uint8_t outByteCnt = 24;

  if (SerialBT.available() == inByteCnt) {
    byte inBuf[inByteCnt];  //受信
    SerialBT.readBytes(inBuf, inByteCnt);

    if (inBuf[0] == 's' || inBuf[0] == 'g') {  //送信
      byte outBuf[outByteCnt];
      outBuf[0] = 's';  // 1 スタートビット
      outBuf[1] = (int16_t)(dt) >> 8;
      outBuf[2] = (int16_t)(dt)&0xFF;
      outBuf[3] = (int16_t)(period) >> 8;
      outBuf[4] = (int16_t)(period)&0xFF;
      outBuf[5] = (int16_t)(batt) >> 8;
      outBuf[6] = (int16_t)(batt)&0xFF;
      outBuf[7] = (int16_t)(angle[0] * 100) >> 8;
      outBuf[8] = (int16_t)(angle[0] * 100) & 0xFF;
      outBuf[9] = (int16_t)(angle[1] * 100) >> 8;
      outBuf[10] = (int16_t)(angle[1] * 100) & 0xFF;
      outBuf[11] = (int16_t)(angle[2] * 100) >> 8;
      outBuf[12] = (int16_t)(angle[2] * 100) & 0xFF;
      outBuf[13] = (int16_t)(motor1_speed) >> 8;
      outBuf[14] = (int16_t)(motor1_speed)&0xFF;
      outBuf[15] = (int16_t)(motor1_speed) >> 8;
      outBuf[16] = (int16_t)(motor1_speed)&0xFF;
      outBuf[17] = (int16_t)(gyroRawLPF.B16.X) >> 8;
      outBuf[18] = (int16_t)(gyroRawLPF.B16.X) & 0xFF;
      outBuf[19] = (int16_t)(gyroRawLPF.B16.Y) >> 8;
      outBuf[20] = (int16_t)(gyroRawLPF.B16.Y) & 0xFF;
      outBuf[21] = (int16_t)(gyroRawLPF.B16.Z) >> 8;
      outBuf[22] = (int16_t)(gyroRawLPF.B16.Z) & 0xFF;
      outBuf[23] = 'e';
      SerialBT.write(outBuf, outByteCnt);

      if (inBuf[0] == 's') {
        int16_t raw[7];
        raw[0] = ((int16_t)inBuf[1] << 8) + (inBuf[2] & 0xff);
        raw[1] = ((int16_t)inBuf[3] << 8) + (inBuf[4] & 0xff);
        raw[2] = ((int16_t)inBuf[5] << 8) + (inBuf[6] & 0xff);
        raw[3] = ((int16_t)inBuf[7] << 8) + (inBuf[8] & 0xff);
        raw[4] = ((int16_t)inBuf[9] << 8) + (inBuf[10] & 0xff);
        raw[5] = ((int16_t)inBuf[11] << 8) + (inBuf[12] & 0xff);
        raw[6] = ((int16_t)inBuf[13] << 8) + (inBuf[14] & 0xff);
        raw[7] = ((int16_t)inBuf[15] << 8) + (inBuf[16] & 0xff);
        raw[8] = ((int16_t)inBuf[17] << 8) + (inBuf[18] & 0xff);
        Kp = raw[0] * 0.1;
        Ki = raw[1] * 0.1;
        Kd = raw[2] * 0.1;
        Ka = raw[3] * 0.1;
        Kt = raw[4] * 0.1;
        rollAdj = raw[5] * 0.01;
        pitchAdj = raw[6] * 0.01;
        yawAdj = raw[7] * 0.1;
        Kz = raw[8] * 0.1;
      }

    } else {  //inBuf[0] == 's'
      while (SerialBT.available() > 0) SerialBT.read();
      //Serial.println("missmatch");
    }
  }  //Serial.available() == inByteCnt

  if (SerialBT.available() > inByteCnt) {  //オーバーフロー
    while (SerialBT.available() > 0) SerialBT.read();
    //Serial.println("overfllow");
  }
}