#include <Wire.h>
#include <BleMouse.h>

// ============ PINS ============
#define SDA_PIN 21
#define SCL_PIN 22
#define BTN_LEFT_PIN 23
#define BTN_RIGHT_PIN 19

// ============ TUNING (change these) ============
float SENS_X = 25.0;        // tilt speed left/right
float SENS_Y = 25.0;        // tilt speed up/down
float ACC_GAIN = 6.0;       // slide push strength (0 = off)
float DEADZONE = 1.5;       // ignore small gyro noise (deg/s)
float ACC_DEADZONE = 0.08;  // ignore small slide (g)
bool SWAP_XY = true;        // true = swap left/right with up/down
bool INVERT_X = false;      // true = reverse left/right
bool INVERT_Y = true;       // true = reverse up/down
bool SLIDE_SWAP = true;     // true = swap slide left/right with up/down
bool ACC_INVERT_X = false;  // true = reverse slide left/right
bool ACC_INVERT_Y = false;  // true = reverse slide up/down

// ============ MPU6050 ============
#define MPU_ADDR 0x68
const float GYRO_LSB = 65.5;    // +-500 dps
const float ACCEL_LSB = 8192.0; // +-4 g

BleMouse bleMouse("ESP32 Air Mouse", "DIY", 100);

float ax, ay, az, gx, gy, gz;
float gbx = 0, gby = 0, gbz = 0;   // gyro bias
float abx = 0, aby = 0;            // accel bias (flat and still)
float remX = 0, remY = 0;
unsigned long lastUs = 0;
bool lastL = false, lastR = false;

void writeReg(byte reg, byte val) {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool readSensor() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(MPU_ADDR, 14) != 14) return false;
  int16_t rax = (Wire.read() << 8) | Wire.read();
  int16_t ray = (Wire.read() << 8) | Wire.read();
  int16_t raz = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();  // temperature
  int16_t rgx = (Wire.read() << 8) | Wire.read();
  int16_t rgy = (Wire.read() << 8) | Wire.read();
  int16_t rgz = (Wire.read() << 8) | Wire.read();
  ax = rax / ACCEL_LSB;
  ay = ray / ACCEL_LSB;
  az = raz / ACCEL_LSB;
  gx = rgx / GYRO_LSB;
  gy = rgy / GYRO_LSB;
  gz = rgz / GYRO_LSB;
  return true;
}

float dead(float v, float dz) {
  if (v > dz) return v - dz;
  if (v < -dz) return v + dz;
  return 0;
}

void setup() {
  Serial.begin(115200);
  pinMode(BTN_LEFT_PIN, INPUT_PULLUP);
  pinMode(BTN_RIGHT_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Wire.beginTransmission(MPU_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("MPU6050 not found. Check wiring.");
    while (true) delay(1000);
  }
  writeReg(0x6B, 0x01);  // wake up
  delay(100);
  writeReg(0x1A, 0x03);  // low-pass filter
  writeReg(0x1B, 0x08);  // gyro +-500 dps
  writeReg(0x1C, 0x08);  // accel +-4 g

  Serial.println("Hold still, calibrating...");
  delay(1000);
  float sgx = 0, sgy = 0, sgz = 0, sax = 0, say = 0;
  int n = 0;
  for (int i = 0; i < 400; i++) {
    if (readSensor()) {
      sgx += gx; sgy += gy; sgz += gz;
      sax += ax; say += ay;
      n++;
    }
    delay(5);
  }
  if (n > 0) {
    gbx = sgx / n; gby = sgy / n; gbz = sgz / n;
    abx = sax / n; aby = say / n;
  }
  Serial.println("Done. Pair with ESP32 Air Mouse.");
  bleMouse.begin();
  lastUs = micros();
}

void loop() {
  unsigned long nowUs = micros();
  if (nowUs - lastUs < 10000) return;  // 100 Hz
  float dt = (nowUs - lastUs) * 1e-6;
  lastUs = nowUs;

  if (!readSensor()) return;

  // ---- Buttons ----
  bool l = (digitalRead(BTN_LEFT_PIN) == LOW);
  bool r = (digitalRead(BTN_RIGHT_PIN) == LOW);
  if (bleMouse.isConnected()) {
    if (l != lastL) { if (l) bleMouse.press(MOUSE_LEFT); else bleMouse.release(MOUSE_LEFT); }
    if (r != lastR) { if (r) bleMouse.press(MOUSE_RIGHT); else bleMouse.release(MOUSE_RIGHT); }
  }
  lastL = l; lastR = r;
  if (!bleMouse.isConnected()) { remX = 0; remY = 0; return; }

  // ---- Gyro tilt ----
  // Pins on the left, chip up, held like a remote:
  //   X gyro = tilting right/left (roll), Y gyro = tipping front up/down (pitch)
  float tiltH = dead(gx - gbx, DEADZONE);   // horizontal (roll)
  float tiltV = dead(gy - gby, DEADZONE);   // vertical (pitch)

  // ---- Accelerometer slide push ----
  float slideH = dead(ay - aby, ACC_DEADZONE);  // sliding left/right
  float slideV = dead(ax - abx, ACC_DEADZONE);  // sliding forward/back
  if (ACC_INVERT_X) slideH = -slideH;
  if (ACC_INVERT_Y) slideV = -slideV;

  float h = tiltH * SENS_X;
  float v = tiltV * SENS_Y;
  float sh = slideH * ACC_GAIN * 10.0;
  float sv = slideV * ACC_GAIN * 10.0;

  if (SWAP_XY) { float t = h; h = v; v = t; }
  if (INVERT_X) h = -h;
  if (INVERT_Y) v = -v;

  // slide gets its own swap, so tilt is not touched
  if (SLIDE_SWAP) { float t = sh; sh = sv; sv = t; }
  h += sh;
  v += sv;

  remX += h * dt;
  remY += v * dt;
  int ix = (int)remX;
  int iy = (int)remY;
  remX -= ix;
  remY -= iy;
  ix = constrain(ix, -127, 127);
  iy = constrain(iy, -127, 127);
  if (ix != 0 || iy != 0) bleMouse.move((signed char)ix, (signed char)iy);
}