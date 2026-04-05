import processing.serial.*;
import controlP5.*;

Serial myPort;

ControlP5   cp5;
Chart       dtChart, AngleChart;
Slider      KtSlider, KaSlider, KpSlider, KiSlider, KdSlider, KzSlider, rollAdjSlider, pitchAdjSlider, yawAdjSlider;
Knob        motor1Knob;

int currentTime, serialTime, updateTime;
int serial_count;
int dt, freq;
float roll, pitch, yaw;
int motor1_speed, az;
float gx, gy, gz, batt;

float Kt=1.3;
float Ka=5.5;
float Kp=32.0;
float Ki=45.0;
float Kd=95.0;
float Kz = 0.0;
float rollAdj=0.0;
float pitchAdj=0.0;
float yawAdj=0.0;

void setup() {
  size(600, 300);
  smooth();
  frameRate(60);

  //serial setup/////////////////////////////////////////////////////////////////////////////////////////////
  myPort = new Serial(this, "COM4", 115200);
  //myPort.buffer(64);
  delay(500);

  //font/////////////////////////////////////////////////////////////////////////////////////////////////////
  String[] fontList = PFont.list();
  printArray(fontList);
  PFont font = createFont("Consolas", 16, true);
  fill(20);
  textAlign(RIGHT);
  textFont(font);
  textSize(17);

  //ControlP5 setup/////////////////////////////////////////////////////////////
  cp5 = new ControlP5(this);
  cp5.setFont(font);
  cp5.addFrameRate().setInterval(10).setColorValueLabel(color(130)) .setPosition(0, height - 20);
  //chart
  dtChart  = cp5.addChart("dt").setPosition(200, 10).setSize(150, 40).setRange(-1, 300).setView(Chart.LINE).setStrokeWeight(5).setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230));
  dtChart.addDataSet("freq") .setColors("freq", color(10, 10, 10))   .setData("freq", new float[300]);
  dtChart.addDataSet("0s") .setColors("0s", color(180, 180, 180))   .setData("0s", new float[300]);

  AngleChart  = cp5.addChart("Angle").setPosition(200, 80).setSize(150, 80).setRange(-3, 3).setView(Chart.LINE).setStrokeWeight(5).setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230));
  AngleChart.addDataSet("0deg") .setColors("0deg", color(180, 180, 180))   .setData("0deg", new float[300]);
  AngleChart.addDataSet("Roll") .setColors("Roll", color(10, 10, 10))   .setData("Roll", new float[300]);
  AngleChart.addDataSet("Pitch") .setColors("Pitch", color(0, 0, 255))   .setData("Pitch", new float[300]);

  //slider
  KtSlider  = cp5.addSlider("Kt").setPosition(390, 10).setSize(15, 50).setRange(0, 100).setNumberOfTickMarks(1001).snapToTickMarks(true).setValue(Kt).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Kt") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);
  KaSlider  = cp5.addSlider("Ka").setPosition(450, 10).setSize(15, 50).setRange(0, 100).setNumberOfTickMarks(1001).snapToTickMarks(true).setValue(Ka).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Ka") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);
  KzSlider  = cp5.addSlider("Kz").setPosition(510, 10).setSize(15, 50).setRange(0, 100).setNumberOfTickMarks(1001).snapToTickMarks(true).setValue(Kz).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Ka") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);

  KpSlider  = cp5.addSlider("Kp").setPosition(390, 80).setSize(15, 50).setRange(0, 100).setNumberOfTickMarks(101).snapToTickMarks(true).setValue(Kp).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Kp") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);
  KiSlider  = cp5.addSlider("Ki").setPosition(450, 80).setSize(15, 50).setRange(0, 100).setNumberOfTickMarks(1001).snapToTickMarks(true).setValue(Ki).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Ki") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);
  KdSlider  = cp5.addSlider("Kd").setPosition(510, 80).setSize(15, 50).setRange(0, 300).setNumberOfTickMarks(3001).snapToTickMarks(true).setValue(Kd).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(1);
  //cp5.getController("Kd") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(10);

  rollAdjSlider  = cp5.addSlider("rollAdj").setPosition(390, 210).setSize(50, 15).setRange(-10, 10).setNumberOfTickMarks(2001).snapToTickMarks(true).setValue(rollAdj).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(2);
  cp5.getController("rollAdj") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(0);
  pitchAdjSlider  = cp5.addSlider("pitchAdj").setPosition(390, 240).setSize(50, 15).setRange(-10, 10).setNumberOfTickMarks(2001).snapToTickMarks(true).setValue(pitchAdj).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(2);
  cp5.getController("pitchAdj") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(0);
  yawAdjSlider  = cp5.addSlider("yawAdj").setPosition(390, 270).setSize(50, 15).setRange(-80, 80).setNumberOfTickMarks(161).snapToTickMarks(true).setValue(pitchAdj).setLock(false) .setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(180, 180, 180)).setColorActive(color(140, 140, 140))  .setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(2);
  cp5.getController("yawAdj") .getCaptionLabel().align(ControlP5.LEFT_OUTSIDE, ControlP5.LEFT_OUTSIDE).setPaddingX(0);

  //knob
  motor1Knob   = cp5.addKnob("motor1").setPosition(220, 200).setRange(0, 1000).setNumberOfTickMarks(10).setValue(0).setRadius(40).setLock(true).setColorCaptionLabel(color(10)).setColorBackground(color(230, 230, 230)).setColorForeground(color(120, 120, 120)).setColorValueLabel(color(10, 10, 10)).setFont(createFont("", 12)).setDecimalPrecision(0);
}

void draw() {
  background(255, 255, 255);
  currentTime = millis();
  if (currentTime - updateTime >20) {
    updateTime = millis();
    update();
    //output.println(updateTime + "," + roll);
    //output.flush();
  }

  serialCom();
  textProcess();
}

void update() {
  //chart update//////////////////////////////////////////////////////////////////////////////////////////////////////////////
  AngleChart.push("Roll", roll);
  AngleChart.push("Pitch", pitch);
  dtChart.push("freq", (1000000.0*1.0)/freq);

  //slider update//////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Kt = KtSlider.getValue();
  Ka = KaSlider.getValue();
  Kp = KpSlider.getValue();
  Ki = KiSlider.getValue();
  Kd = KdSlider.getValue();
  Kz = KzSlider.getValue();
  rollAdj = rollAdjSlider.getValue();
  pitchAdj = pitchAdjSlider.getValue();
  yawAdj = yawAdjSlider.getValue();

  //knob update///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  motor1Knob.setValue(abs(motor1_speed));
}

void textProcess() {
  fill(10);
  textSize(17);
  TEXT1(20, 20, "dt", dt/1000.0, 1, "ms", 120);
  TEXT1(20, 40, "freq", (1000000.0)/freq, 1, "Hz", 120);
  TEXT1(20, 60, "batt", batt, 2, "V", 120);
  TEXT3(20, 100, "roll", "pitch", "yaw", roll, pitch, yaw, 1, "deg", 120);
  TEXT3(20, 180, "gx", "gy", "gz", gx, gy, gz, 2, "deg/s", 90);
}

void TEXT1(int x, int y, String text1, float a, int dec, String text2, int clearance) {
  fill(20);

  textAlign(LEFT);
  textSize(17);
  text(text1, x, y);
  textAlign(RIGHT);
  text(nf(a, 1, dec), x+clearance, y);
  textAlign(LEFT);
  text(text2, x+clearance+10, y);
}

void TEXT3(int x, int y, String text1, String text2, String text3, float a, float b, float c, int dec, String text4, int clearance) {
  fill(20);
  textSize(17);

  textAlign(LEFT);
  text(text1, x, y);
  text(text2, x, y+20);
  text(text3, x, y+40);

  textAlign(RIGHT);
  text(nf(a, 1, dec), x+clearance, y);
  text(nf(b, 1, dec), x+clearance, y+20);
  text(nf(c, 1, dec), x+clearance, y+40);

  textAlign(LEFT);
  text(text4, x+clearance+10, y);
  text(text4, x+clearance+10, y+20);
  text(text4, x+clearance+10, y+40);
}

int inByteCnt = 24;
byte[] inBuf = new byte[inByteCnt];

void serialCom() {
  while (myPort.available() > 0) {
    byte data = (byte)myPort.read();
    if (serial_count < inByteCnt) {
      inBuf[serial_count] = data;
      serial_count ++;
    }
    if (serial_count == inByteCnt && inBuf[0]=='s' && inBuf[inByteCnt-1]=='e') {
      dt    = (inBuf[1]<<8)+(inBuf[2]&0xFF);
      freq  = (inBuf[3]<<8)+(inBuf[4]&0xFF);
      batt  = (inBuf[5]<<8)+(inBuf[6]&0xFF);
      roll  = (inBuf[7]<<8)+(inBuf[8]&0xFF);
      pitch = (inBuf[9]<<8)+(inBuf[10]&0xFF);
      yaw   = (inBuf[11]<<8)+(inBuf[12]&0xFF);
      motor1_speed = (inBuf[13]<<8)+(inBuf[14]&0xFF);
      az    = (inBuf[15]<<8)+(inBuf[16]&0xFF);
      gx    = (inBuf[17]<<8)+(inBuf[18]&0xFF);
      gy    = (inBuf[19]<<8)+(inBuf[20]&0xFF);
      gz    = (inBuf[21]<<8)+(inBuf[22]&0xFF);

      roll  /=100.0;
      pitch /=100.0;
      yaw   /=100.0;
      gx    /=65.536;
      gy    /=65.536;
      gz    /=65.536;
      batt = batt/74.28571429;
    }
  }

  if (currentTime - serialTime > 50) {
    serialTime = millis();
    if (serial_count==0) {
      println("NO DATA");
    } else if (serial_count < inByteCnt) {
      println("TIMEOUT");
    } else if (serial_count > inByteCnt) {
      println("OVERRUN");
    }

    byte[] outBuf = new byte[19];
    outBuf[0]  = 's';
    outBuf[1]  = (byte)(((int)(Kp*10)>>8) & 0x00ff);
    outBuf[2]  = (byte)((int)(Kp*10) & 0x00ff);
    outBuf[3]  = (byte)(((int)(Ki*10)>>8) & 0x00ff);
    outBuf[4]  = (byte)((int)(Ki*10) & 0x00ff);
    outBuf[5]  = (byte)(((int)(Kd*10)>>8) & 0x00ff);
    outBuf[6]  = (byte)((int)(Kd*10) & 0x00ff);
    outBuf[7]  = (byte)(((int)(Ka*10)>>8) & 0x00ff);
    outBuf[8]  = (byte)((int)(Ka*10) & 0x00ff);
    outBuf[9]  = (byte)(((int)(Kt*10)>>8) & 0x00ff);
    outBuf[10] = (byte)((int)(Kt*10) & 0x00ff);
    outBuf[11]  = (byte)(((int)(rollAdj*100)>>8) & 0x00ff);
    outBuf[12] = (byte)((int)(rollAdj*100) & 0x00ff);
    outBuf[13]  = (byte)(((int)(pitchAdj*100)>>8) & 0x00ff);
    outBuf[14] = (byte)((int)(pitchAdj*100) & 0x00ff);
    outBuf[15]  = (byte)(((int)(yawAdj*10)>>8) & 0x00ff);
    outBuf[16] = (byte)((int)(yawAdj*10) & 0x00ff);
    outBuf[17]  = (byte)(((int)(Kz*10)>>8) & 0x00ff);
    outBuf[18] = (byte)((int)(Kz*10) & 0x00ff);
    myPort.write(outBuf);
    serial_count = 0;
  }
}
