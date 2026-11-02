/*
  SINGLE MPU6050 WIRING TEST — MPU6050_tockn version
  This library talks directly to the sensor registers with no
  chip-ID/WHO_AM_I validation step, so clone boards work fine.

  ESP32 DevKit V1 + one MPU6050 at 0x68 (AD0 -> GND, the address
  this library expects by default)
  SDA = GPIO21, SCL = GPIO22

  Library needed: "MPU6050_tockn" (search "tockn" in Library Manager)
*/

#include <Wire.h>
#include <MPU6050_tockn.h>

#define SDA_PIN 21
#define SCL_PIN 22

MPU6050 mpu6050(Wire);

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("=== Single MPU6050 wiring test (tockn library) ===");

  Wire.begin(SDA_PIN, SCL_PIN);

  // Quick scan just to confirm the address, same as before
  Serial.println("Scanning I2C bus...");
  bool found = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.println(addr, HEX);
      if (addr == 0x68) found = true;
    }
  }
  if (!found) {
    Serial.println("ERROR: 0x68 not found. Check VCC, GND, SDA, SCL, AD0 wiring.");
  }

  mpu6050.begin();

  Serial.println("Calibrating gyro \u2014 keep the sensor COMPLETELY STILL now...");
  mpu6050.calcGyroOffsets(true); // prints progress dots while it does this
  Serial.println("Calibration done.");

  Serial.println("Starting readings (accel m/s^2, gyro deg/s)...");
  Serial.println("ax,ay,az,gx,gy,gz");
}

void loop() {
  mpu6050.update();

  const float G_TO_MS2 = 9.80665;
  float ax_ms2 = mpu6050.getAccX() * G_TO_MS2;
  float ay_ms2 = mpu6050.getAccY() * G_TO_MS2;
  float az_ms2 = mpu6050.getAccZ() * G_TO_MS2;
  float gx_dps = mpu6050.getGyroX();
  float gy_dps = mpu6050.getGyroY();
  float gz_dps = mpu6050.getGyroZ();

  Serial.print(ax_ms2, 3); Serial.print(",");
  Serial.print(ay_ms2, 3); Serial.print(",");
  Serial.print(az_ms2, 3); Serial.print(",");
  Serial.print(gx_dps, 3); Serial.print(",");
  Serial.print(gy_dps, 3); Serial.print(",");
  Serial.println(gz_dps, 3);

  delay(50); // ~20 Hz, plenty for a wiring checkx`
}