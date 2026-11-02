/*
  ================================================================
  KNEE OA IMU DATA LOGGER — STAGE 1: RAW ACQUISITION
  ================================================================
  Hardware:
    - ESP32 DevKit V1 (ESP32-WROOM)
    - MPU6050 #1 "THIGH" on I2C address 0x68 (AD0 -> GND)
    - MPU6050 #2 "SHIN"  on I2C address 0x69 (AD0 -> 3.3V)
    - Both sensors share ONE I2C bus: SDA = GPIO 21, SCL = GPIO 22

  Output: CSV over USB serial at 115200 baud, ~100 Hz.
  Header:
    sample,timestamp_ms,thigh_ax,thigh_ay,thigh_az,thigh_gx,thigh_gy,thigh_gz,
    shin_ax,shin_ay,shin_az,shin_gx,shin_gy,shin_gz

  Units:
    Acceleration: m/s^2
    Angular velocity: deg/s

  Explicitly NOT implemented in this stage (by design):
    knee angle, Euler/quaternion fusion, Madgwick/Mahony/Kalman filters,
    ROM, gait/step/sit-to-stand/squat detection, ML/classification,
    Wi-Fi/Bluetooth/web server/mobile app.
  These come in later stages, on top of this raw data stream.
  ================================================================
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ---------------- Configuration ----------------
#define SDA_PIN 21
#define SCL_PIN 22

#define THIGH_ADDR 0x68
#define SHIN_ADDR  0x69

#define SAMPLE_PERIOD_US 10000UL   // 100 Hz -> 10 ms -> 10000 us

#define DEBUG_MODE 0               // 1 = extra human-readable prints mixed with clear markers, 0 = clean CSV only

#define CALIBRATE_ON_STARTUP 1     // set to 0 to skip gyro bias calibration
#define CALIBRATION_SAMPLES 200    // ~2 seconds at 100 Hz

// ---------------- Globals ----------------
Adafruit_MPU6050 thighSensor;
Adafruit_MPU6050 shinSensor;

bool thighOK = false;
bool shinOK = false;

uint32_t sampleIndex = 0;
uint64_t nextSampleMicros = 0;
uint32_t startMillis = 0;

// Gyro bias (deg/s), applied as: corrected = raw - bias
float thighGyroBias[3] = {0, 0, 0};
float shinGyroBias[3]  = {0, 0, 0};

// ---------------- Function prototypes ----------------
void scanI2C();
bool initializeSensors();
void calibrateGyroscope();
void printCSVHeader();
void readAndPrintSample();

// ================================================================
void setup() {
  Serial.begin(115200);
  delay(300); 

  Serial.println();
  Serial.println("================================");
  Serial.println("KNEE OA IMU DATA LOGGER");
  Serial.println("================================");
  Serial.println();
  Serial.println("Initializing I2C...");
  Serial.println("SDA: GPIO 21");
  Serial.println("SCL: GPIO 22");
  Serial.println();

  Wire.begin(SDA_PIN, SCL_PIN);   // explicit pins, never the default Wire.begin()

  scanI2C();

  bool sensorsReady = initializeSensors();

  if (!sensorsReady) {
    Serial.println();
    Serial.println("One or both sensors failed to initialize. Halting.");
    Serial.println("Fix wiring/power and reset the ESP32.");
    while (true) {
      delay(1000); // stop here — do not silently continue as if working
    }
  }

#if CALIBRATE_ON_STARTUP
  calibrateGyroscope();
#endif

  Serial.println();
  Serial.println("Starting data acquisition...");
  Serial.println("================================");
  Serial.println();

  printCSVHeader();

  startMillis = millis();
  nextSampleMicros = micros();
}

// ================================================================
void loop() {
  uint64_t nowMicros = micros();

  if ((int64_t)(nowMicros - nextSampleMicros) >= 0) {
    readAndPrintSample();
    sampleIndex++;
    nextSampleMicros += SAMPLE_PERIOD_US;

    // If we fell far behind (e.g. during calibration/startup jitter),
    // resync instead of trying to "catch up" with a burst of samples.
    if ((int64_t)(nowMicros - nextSampleMicros) > (int64_t)SAMPLE_PERIOD_US) {
      nextSampleMicros = nowMicros + SAMPLE_PERIOD_US;
    }
  }
}

// ================================================================
void scanI2C() {
  Serial.println("Scanning I2C bus...");
  Serial.println();

  bool found68 = false;
  bool found69 = false;

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      if (addr == THIGH_ADDR) found68 = true;
      if (addr == SHIN_ADDR)  found69 = true;
    }
  }

  Serial.println();
  if (!found68) {
    Serial.println("ERROR: THIGH MPU6050 at 0x68 not detected.");
  }
  if (!found69) {
    Serial.println("ERROR: SHIN MPU6050 at 0x69 not detected.");
  }
  Serial.println();
}

// ================================================================
bool initializeSensors() {
  // THIGH @ 0x68
  thighOK = thighSensor.begin(THIGH_ADDR, &Wire, 0);
  if (thighOK) {
    // Ranges appropriate for walking / knee bending / sit-to-stand /
    // squat-like movement — not extreme-impact sports:
    //  - Accel: +/- 4g covers normal limb accelerations with headroom,
    //    without sacrificing resolution the way +/-16g would.
    //  - Gyro: +/- 500 deg/s comfortably covers knee flexion/extension
    //    angular rates during normal daily activities.
    //  - DLPF ~44 Hz (BAND_44_HZ) removes high-frequency vibration/noise
    //    while preserving the low-frequency content of human movement
    //    (well above the 100 Hz sample rate's Nyquist-relevant content).
    thighSensor.setAccelerometerRange(MPU6050_RANGE_4_G);
    thighSensor.setGyroRange(MPU6050_RANGE_500_DEG);
    thighSensor.setFilterBandwidth(MPU6050_BAND_44_HZ);
    Serial.println("THIGH MPU6050 [0x68]: OK");
  } else {
    Serial.println("THIGH MPU6050 [0x68]: FAILED (address seen but init failed, or not present)");
  }

  // SHIN @ 0x69
  shinOK = shinSensor.begin(SHIN_ADDR, &Wire, 1);
  if (shinOK) {
    shinSensor.setAccelerometerRange(MPU6050_RANGE_4_G);
    shinSensor.setGyroRange(MPU6050_RANGE_500_DEG);
    shinSensor.setFilterBandwidth(MPU6050_BAND_44_HZ);
    Serial.println("SHIN  MPU6050 [0x69]: OK");
  } else {
    Serial.println("SHIN  MPU6050 [0x69]: FAILED (address seen but init failed, or not present)");
  }

  return thighOK && shinOK;
}

void calibrateGyroscope() {
  Serial.println();
  Serial.println("Gyro bias calibration:");
  Serial.println("Keep BOTH sensors completely still...");
  delay(1500);

  double sumThigh[3] = {0, 0, 0};
  double sumShin[3]  = {0, 0, 0};

  sensors_event_t a, g, temp;

  for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
    thighSensor.getEvent(&a, &g, &temp);
    sumThigh[0] += g.gyro.x * RAD_TO_DEG;
    sumThigh[1] += g.gyro.y * RAD_TO_DEG;
    sumThigh[2] += g.gyro.z * RAD_TO_DEG;

    shinSensor.getEvent(&a, &g, &temp);
    sumShin[0] += g.gyro.x * RAD_TO_DEG;
    sumShin[1] += g.gyro.y * RAD_TO_DEG;
    sumShin[2] += g.gyro.z * RAD_TO_DEG;

    delay(5);
  }

  for (int i = 0; i < 3; i++) {
    thighGyroBias[i] = sumThigh[i] / CALIBRATION_SAMPLES;
    shinGyroBias[i]  = sumShin[i]  / CALIBRATION_SAMPLES;
  }

  Serial.println("Calibration complete.");
  Serial.print("THIGH gyro bias (deg/s): ");
  Serial.print(thighGyroBias[0]); Serial.print(", ");
  Serial.print(thighGyroBias[1]); Serial.print(", ");
  Serial.println(thighGyroBias[2]);
  Serial.print("SHIN  gyro bias (deg/s): ");
  Serial.print(shinGyroBias[0]); Serial.print(", ");
  Serial.print(shinGyroBias[1]); Serial.print(", ");
  Serial.println(shinGyroBias[2]);
}

// ================================================================
void printCSVHeader() {
  Serial.println("sample,timestamp_ms,thigh_ax,thigh_ay,thigh_az,thigh_gx,thigh_gy,thigh_gz,shin_ax,shin_ay,shin_az,shin_gx,shin_gy,shin_gz");
}

// ================================================================
void readAndPrintSample() {
  sensors_event_t thighAccel, thighGyro, thighTemp;
  sensors_event_t shinAccel, shinGyro, shinTemp;

  // Sequential reads: THIGH first, then SHIN. At 100 Hz (10 ms period),
  // the I2C read of each sensor takes on the order of a few hundred
  // microseconds, so the time offset between the two reads is roughly
  // 1-3% of the sample period — negligible for this stage, but worth
  // knowing about if later stages need tighter cross-sensor timing.
  thighSensor.getEvent(&thighAccel, &thighGyro, &thighTemp);
  shinSensor.getEvent(&shinAccel, &shinGyro, &shinTemp);

  float thigh_gx = (thighGyro.gyro.x * RAD_TO_DEG) - thighGyroBias[0];
  float thigh_gy = (thighGyro.gyro.y * RAD_TO_DEG) - thighGyroBias[1];
  float thigh_gz = (thighGyro.gyro.z * RAD_TO_DEG) - thighGyroBias[2];

  float shin_gx = (shinGyro.gyro.x * RAD_TO_DEG) - shinGyroBias[0];
  float shin_gy = (shinGyro.gyro.y * RAD_TO_DEG) - shinGyroBias[1];
  float shin_gz = (shinGyro.gyro.z * RAD_TO_DEG) - shinGyroBias[2];

  uint32_t timestamp_ms = millis() - startMillis;

#if DEBUG_MODE
  Serial.print("DEBUG: loop time us=");
  Serial.println(micros());
#endif

  Serial.print(sampleIndex);        Serial.print(",");
  Serial.print(timestamp_ms);       Serial.print(",");
  Serial.print(thighAccel.acceleration.x, 4); Serial.print(",");
  Serial.print(thighAccel.acceleration.y, 4); Serial.print(",");
  Serial.print(thighAccel.acceleration.z, 4); Serial.print(",");
  Serial.print(thigh_gx, 4); Serial.print(",");
  Serial.print(thigh_gy, 4); Serial.print(",");
  Serial.print(thigh_gz, 4); Serial.print(",");
  Serial.print(shinAccel.acceleration.x, 4); Serial.print(",");
  Serial.print(shinAccel.acceleration.y, 4); Serial.print(",");
  Serial.print(shinAccel.acceleration.z, 4); Serial.print(",");
  Serial.print(shin_gx, 4); Serial.print(",");
  Serial.print(shin_gy, 4); Serial.print(",");
  Serial.println(shin_gz, 4);
}
