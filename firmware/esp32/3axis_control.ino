//2024/08/10
//3軸姿勢制御モジュール Ver1.0.0
//imo Lab. https://Garchiving.com
//https://garchiving.com/summary-of-self-balancing-cube/

#include <Wire.h>
#include "BluetoothSerial.h"

#define _Ks 1.0
#define _Kp_angle 5.0
#define _Kp 60.0
#define _Ki 12.0
#define _Kd 90.0

#define _ROLL_ADJUST -5.2
#define _PITCH_ADJUST 0.5

#define ROLL 0
#define PITCH 1
#define YAW 2

#define PIN_PWM_CTR 25
#define PIN_BRAKE_CTR 17
#define PIN_DIRECTION_CTR 13

#define PIN_PWM_RH 26
#define PIN_BRAKE_RH 18
#define PIN_DIRECTION_RH 14

#define PIN_PWM_LH 27
#define PIN_BRAKE_LH 19
#define PIN_DIRECTION_LH 16

BluetoothSerial SerialBT;

int16_t voltage_mV, current_mA;
uint32_t currentTime, mainLoopTimer, BluetoothTimer, dt, dtTime;

int16_t PID[2];

bool vertical;

float TargetOmega[2], TargetAngle[2];
float P[2], preP[2], I[2], D[2];
float Ks, Kp_angle, Kp, Ki, Kd;
float ROLL_ADJUST, PITCH_ADJUST;

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
  SerialBT.begin("ESP32-CUBE");

  Wire.begin();
  Wire.setClock(400000);
  delay(500);

  pinMode(PIN_BRAKE_CTR, OUTPUT);
  pinMode(PIN_BRAKE_RH, OUTPUT);
  pinMode(PIN_BRAKE_LH, OUTPUT);

  pinMode(PIN_DIRECTION_CTR, OUTPUT);
  pinMode(PIN_DIRECTION_RH, OUTPUT);
  pinMode(PIN_DIRECTION_LH, OUTPUT);

  digitalWrite(PIN_BRAKE_CTR, LOW);
  digitalWrite(PIN_BRAKE_RH, LOW);
  digitalWrite(PIN_BRAKE_LH, LOW);

  digitalWrite(PIN_DIRECTION_CTR, HIGH);
  digitalWrite(PIN_DIRECTION_RH, HIGH);
  digitalWrite(PIN_DIRECTION_LH, HIGH);

  ledcAttach(PIN_PWM_CTR, 20000, 8);
  ledcWrite(PIN_PWM_CTR, 255);

  ledcAttach(PIN_PWM_RH, 20000, 8);
  ledcWrite(PIN_PWM_RH, 255);

  ledcAttach(PIN_PWM_LH, 20000, 8);
  ledcWrite(PIN_PWM_LH, 255);

  setupINA219();
  setupMPU6050();
  readMPU6050();
  Vg.B16.X = accRaw.B16.X;
  Vg.B16.Y = accRaw.B16.Y;
  Vg.B16.Z = accRaw.B16.Z;
  accRawLPF.B16.X = accRaw.B16.X;
  accRawLPF.B16.Y = accRaw.B16.Y;
  accRawLPF.B16.Z = accRaw.B16.Z;

  Ks = _Ks;
  Kp_angle = _Kp_angle;
  Kp = _Kp;
  Ki = _Ki;
  Kd = _Kd;
  ROLL_ADJUST = _ROLL_ADJUST;
  PITCH_ADJUST = _PITCH_ADJUST;

  currentTime = micros();
  mainLoopTimer = currentTime;
}

void loop() {
  currentTime = micros();

  if (currentTime - mainLoopTimer >= 10000) {  //10ms
    mainLoopTimer = micros();

    readINA219();    //1200us
    computeAngle();  //524us
    angle[ROLL] += ROLL_ADJUST;
    angle[PITCH] += PITCH_ADJUST;

    if (abs(angle[ROLL]) < 1.00 && abs(angle[PITCH]) < 1.00) vertical = 1;
    if (abs(angle[ROLL]) > 7.0 || abs(angle[PITCH]) > 7.00) vertical = 0;

    PID_COMP();  //20us

    Serial.print("dt:");
    Serial.print((float)dt, 0);
    Serial.print(",");
    Serial.print("Ks:");
    Serial.print((float)Ks, 1);
    Serial.print(",");
    Serial.print("Kp:");
    Serial.print((float)Kp, 1);
    Serial.print(",");
    Serial.print("Ki:");
    Serial.print((float)Ki, 1);
    Serial.print(",");
    Serial.print("Kd:");
    Serial.print((float)Kd, 1);
    Serial.print(",");
    Serial.print("roll:");
    Serial.print((float)ROLL_ADJUST, 1);
    Serial.print(",");
    Serial.print("pitch:");
    Serial.print((float)PITCH_ADJUST, 1);
    Serial.print("\n");

    dt = micros() - mainLoopTimer;
  }

  if (currentTime - BluetoothTimer >= 0) {  //120us
    BluetoothTimer = micros();
    BlueToothCom();
  }
}

inline void PID_COMP() {
  if (vertical) {
    //角速度PID
    //目標値補正
    TargetAngle[ROLL] += 0.01 * Ks * (TargetAngle[ROLL] - angle[ROLL]);
    TargetAngle[ROLL] = constrain(TargetAngle[ROLL], -19, 19);

    TargetAngle[PITCH] += 0.01 * Ks * (TargetAngle[PITCH] - angle[PITCH]);
    TargetAngle[PITCH] = constrain(TargetAngle[PITCH], -19, 19);

    //目標角速度
    TargetOmega[ROLL] = Kp_angle * (TargetAngle[ROLL] - angle[ROLL]);
    P[ROLL] = TargetOmega[ROLL] - (gyroRawLPF.B16.X * 0.0152587890625);
    I[ROLL] += P[ROLL] * 0.01;
    I[ROLL] = constrain(I[ROLL], -3, 3);
    D[ROLL] = (gyroRawLPF.B16.X * 0.0152587890625) - preP[ROLL];
    preP[ROLL] = (gyroRawLPF.B16.X * 0.0152587890625);
    PID[ROLL] = Kp * P[ROLL] + Ki * I[ROLL] - Kd * D[ROLL];

    TargetOmega[PITCH] = Kp_angle * (TargetAngle[PITCH] - angle[PITCH]);
    P[PITCH] = TargetOmega[PITCH] - (gyroRawLPF.B16.Y * 0.0152587890625);
    I[PITCH] += P[PITCH] * 0.01;
    I[PITCH] = constrain(I[PITCH], -3, 3);
    D[PITCH] = (gyroRawLPF.B16.Y * 0.0152587890625) - preP[PITCH];
    preP[PITCH] = (gyroRawLPF.B16.Y * 0.0152587890625);
    PID[PITCH] = Kp * P[PITCH] + Ki * I[PITCH] - Kd * D[PITCH];

    PID[ROLL] = constrain(PID[ROLL], -255, 255);
    PID[PITCH] = constrain(PID[PITCH], -255, 255);
    motorControl_CTR(PID[ROLL]);
    motorControl_RH(0.866 * PID[PITCH] + 0.5 * PID[ROLL]);
    motorControl_LH(0.866 * PID[PITCH] - 0.5 * PID[ROLL]);
  } else {
    digitalWrite(PIN_BRAKE_CTR, LOW);
    digitalWrite(PIN_BRAKE_RH, LOW);
    digitalWrite(PIN_BRAKE_LH, LOW);

    I[ROLL] = 0;
    TargetOmega[ROLL] = 0;
    TargetAngle[ROLL] = 0;
    PID[ROLL] = 0;

    I[PITCH] = 0;
    TargetOmega[PITCH] = 0;
    TargetAngle[PITCH] = 0;
    PID[PITCH] = 0;
  }
}

inline void motorControl_CTR(int16_t val) {
  bool motorDir = 1;
  if (val > 0) motorDir = 0;
  if (val < 0) motorDir = 1;

  digitalWrite(PIN_BRAKE_CTR, HIGH);
  digitalWrite(PIN_DIRECTION_CTR, motorDir);
  ledcWrite(PIN_PWM_CTR, 255 - abs(val));
}

inline void motorControl_RH(int16_t val) {
  bool motorDir = 1;
  if (val > 0) motorDir = 1;
  if (val < 0) motorDir = 0;

  digitalWrite(PIN_BRAKE_RH, HIGH);
  digitalWrite(PIN_DIRECTION_RH, motorDir);
  ledcWrite(PIN_PWM_RH, 255 - abs(val));
}

inline void motorControl_LH(int16_t val) {
  bool motorDir = 1;
  if (val > 0) motorDir = 0;
  if (val < 0) motorDir = 1;

  digitalWrite(PIN_BRAKE_LH, HIGH);
  digitalWrite(PIN_DIRECTION_LH, motorDir);
  ledcWrite(PIN_PWM_LH, 255 - abs(val));
}

// ****************************************************************
// *** MPU6050 値読み取り ***//580us
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
// *** 姿勢角度算出 *** UNO:1050ms  ESP32:524us
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
  angle[YAW] += -1 * gyroRaw.B16.Z * 0.0152587890625 * dt2;  //1/LSB
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
  accCal[0] += 1672;
  accCal[1] += -2896;
  accCal[2] += 1731;

  //CUBE用（2軸用）
  //accCal[0] += 1672;
  //accCal[1] += 2896;
  //accCal[2] += 1731;
}

// ****************************************************************
// *** INA219 読み取り ***
// ****************************************************************
inline void readINA219() {
  uint16_t busvoltage_raw, shuntvoltage_raw;
  int16_t busvoltage_mV, shuntvoltage_mV;

  Wire.beginTransmission(0x40);  // デバイスアドレス
  Wire.write(0x02);              // busvoltage_mV_mV_mV_mV_mVレジスタ
  Wire.endTransmission();
  Wire.requestFrom(0x40, 16);
  while (Wire.available() < 16) {};
  busvoltage_raw = Wire.read() << 8 | Wire.read();
  busvoltage_mV = (int16_t)((busvoltage_raw >> 3) * 4);  // mV

  Wire.beginTransmission(0x40);  // デバイスアドレス
  Wire.write(0x01);              // Shuntvoltage_mV_mV_mV_mVレジスタ
  Wire.endTransmission();
  Wire.requestFrom(0x40, 16);
  while (Wire.available() < 16) {};
  shuntvoltage_raw = Wire.read() << 8 | Wire.read();
  shuntvoltage_mV = (int16_t)(shuntvoltage_raw)*0.01;  // *0.01[mV]  // *10[uV]
  current_mA = (int16_t)(shuntvoltage_raw)*0.1;        //I=V/R=V/0.1=V*10

  voltage_mV = busvoltage_mV + shuntvoltage_mV;  // mV
}

// ****************************************************************
// *** INA219 初期設定 ***
// ****************************************************************
void setupINA219() {
  int16_t config_value = 0x399F;  //INA219 デフォルト
  Wire.beginTransmission(0x40);   //INA219 デバイスアドレス
  Wire.write(0x00);               //configアドレス
  Wire.write((config_value >> 8) & 0xFF);
  Wire.write(config_value & 0xFF);
  Wire.endTransmission();
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
  uint8_t inByteCnt = 15;
  uint8_t outByteCnt = 24;

  if (SerialBT.available() == inByteCnt) {
    byte inBuf[inByteCnt];  //受信
    SerialBT.readBytes(inBuf, inByteCnt);

    if (inBuf[0] == 's' || inBuf[0] == 'g') {  //送信
      byte outBuf[outByteCnt];
      outBuf[0] = 's';                                // 1 スタートビット
      outBuf[1] = (int16_t)accRawLPF.B16.X >> 8;      // 2
      outBuf[2] = (int16_t)accRawLPF.B16.X & 0xFF;    // 3
      outBuf[3] = (int16_t)accRawLPF.B16.Y >> 8;      // 4
      outBuf[4] = (int16_t)accRawLPF.B16.Y & 0xFF;    // 5
      outBuf[5] = (int16_t)accRawLPF.B16.Z >> 8;      // 6
      outBuf[6] = (int16_t)accRawLPF.B16.Z & 0xFF;    // 7
      outBuf[7] = (int16_t)gyroRawLPF.B16.X >> 8;     // 8
      outBuf[8] = (int16_t)gyroRawLPF.B16.X & 0xFF;   // 9
      outBuf[9] = (int16_t)gyroRawLPF.B16.Y >> 8;     //10
      outBuf[10] = (int16_t)gyroRawLPF.B16.Y & 0xFF;  //11
      outBuf[11] = (int16_t)gyroRawLPF.B16.Z >> 8;    //12
      outBuf[12] = (int16_t)gyroRawLPF.B16.Z & 0xFF;  //13
      outBuf[13] = (int16_t)(angle[0] * 100) >> 8;    //14
      outBuf[14] = (int16_t)(angle[0] * 100) & 0xFF;  //15
      outBuf[15] = (int16_t)(angle[1] * 100) >> 8;    //16
      outBuf[16] = (int16_t)(angle[1] * 100) & 0xFF;  //17
      outBuf[17] = (int16_t)(angle[2] * 100) >> 8;    //18
      outBuf[18] = (int16_t)(angle[2] * 100) & 0xFF;  //19
      outBuf[19] = (int16_t)voltage_mV >> 8;          // 20
      outBuf[20] = (int16_t)voltage_mV & 0xFF;        // 21
      outBuf[21] = (int16_t)current_mA >> 8;          // 22
      outBuf[22] = (int16_t)current_mA & 0xFF;        // 23
      outBuf[23] = 'e';                               //24 end bit
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

        Ks = raw[0] * 0.1;
        Kp = raw[1] * 0.1;
        Ki = raw[2] * 0.1;
        Kd = raw[3] * 0.1;
        Kp_angle = raw[4] * 0.1;
        ROLL_ADJUST = raw[5] * 0.1;
        PITCH_ADJUST = raw[6] * 0.1;
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
