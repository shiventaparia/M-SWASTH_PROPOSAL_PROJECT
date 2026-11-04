# KneeMotion AI — M-Swasth Objective Health Measurement Platform

A nurse-operated IoT measurement concept for community knee assessment, built as the first use case of a reusable healthcare measurement platform.

**Author:** Shiven Taparia  
**Target partner:** M-Swasth  
**Primary use case:** Knee functional measurement and AI-assisted risk stratification  
**Current stage:** Engineering prototype / research validation

---

## 1. What is KneeMotion?

KneeMotion is designed to turn a short, standardized movement test into structured digital measurements that can be reviewed remotely and tracked over time.

The initial concept uses two wearable inertial sensors:

- **Thigh IMU** — lower-limb motion
- **Shin IMU** — lower-limb motion
- **ESP32** — sensor acquisition and timestamped data capture

The proposed functional protocol includes:

1. Seated knee flexion / extension
2. Five-times sit-to-stand
3. Short timed walk

Target outputs include:

- Knee range of motion (ROM)
- Angular velocity / movement speed
- Sit-to-stand timing
- Gait timing and speed
- Cadence and stance/swing features where validated
- Left-right symmetry where applicable
- Session/data-quality flags

The core product idea is:

> **Measure → Analyze → Risk-stratify → Act**

KneeMotion is intended to become a measurement and decision-support layer inside a distributed care workflow, rather than a standalone consumer gadget.

---

## 2. Why this project exists

M-Swasth already connects patients, local healthcare staff and remote clinicians through a distributed care model. Knee function is difficult to communicate objectively through a remote consultation when the only available information is symptoms and visual observation.

KneeMotion is intended to add a structured movement measurement layer:

```text
Patient
  ↓
Nurse-led guided test
  ↓
Wearable movement capture
  ↓
Signal processing + quality checks
  ↓
Functional measurements
  ↓
Risk / triage support
  ↓
Appropriate care pathway
```

The long-term platform vision is to reuse the same digital foundation for additional validated applications such as balance/fall-risk assessment, diabetic-foot monitoring, shoulder/spine assessment, cardiovascular sensing and respiratory monitoring.

---

## 3. Current engineering status

The repository currently contains two main hardware paths:

### Dual-IMU acquisition prototype

`Code_esp32.ino` implements the current Stage-1 raw acquisition path for **two MPU6050 sensors**:

- Thigh sensor: I²C `0x68`
- Shin sensor: I²C `0x69`
- ESP32 SDA: GPIO 21
- ESP32 SCL: GPIO 22
- Sampling target: **100 Hz**
- Gyroscope bias calibration at startup
- CSV output over USB serial
- Sample index + timestamp

This stage intentionally focuses on reliable acquisition rather than claiming a finished clinical algorithm.

### Single-sensor wiring test

`knee_oa_imu_logger.ino` is a simple one-MPU6050 wiring and sensor test using the `MPU6050_tockn` library. It is useful for checking sensor power, I²C connectivity, calibration and raw acceleration/gyro readings before running the dual-sensor logger.

---

## 4. Hardware architecture

```text
             ┌─────────────────────┐
             │     Thigh IMU       │
             │    MPU6050 @ 0x68   │
             └──────────┬──────────┘
                        │ I²C
                        │
             ┌──────────▼──────────┐
             │       ESP32         │
             │  Acquisition / CSV  │
             └──────────▲──────────┘
                        │ I²C
             ┌──────────┴──────────┐
             │      Shin IMU       │
             │    MPU6050 @ 0x69   │
             └─────────────────────┘
                        │
                        ▼
                USB serial / host
```

The current prototype is deliberately simple: establish synchronized sensing and repeatable acquisition before adding more sensors or complexity.

---

## 5. Data pipeline

The intended processing pipeline is:

```text
Raw IMU samples
      ↓
Packet / quality checks
      ↓
Sensor calibration
      ↓
Orientation / sensor fusion
      ↓
Thigh–shin relative motion
      ↓
Task-specific feature extraction
      ↓
Quality flagging
      ↓
Clinical / research analytics
      ↓
Structured report
```

Important design principle:

> **The device records. The versioned pipeline interprets. The care team makes the decision.**

---

## 6. Machine-learning / clinical analytics

`Model_trial.py` contains an early **XGBoost research pipeline** based on clinical variables such as:

- Age
- BMI
- Joint side/site
- Morning stiffness
- Pain score
- Mechanical pain relieved by rest
- Crepitus
- Squat difficulty
- Stair difficulty
- Sit-to-stand difficulty

The repository also contains `knee_clinical_ml_ready.csv`, a clinical dataset used during early model experimentation.

### Important status

The current model is **not clinically validated** and should not be presented as clinical performance evidence.

The present target is constructed from the same clinical features supplied to the model, which creates label leakage / circularity. The model should therefore be treated as a software research starting point, not as a deployable diagnostic or triage model.

The next ML stage is intended to use an independently defined outcome and held-out participant-level validation, followed by prospective clinical evaluation.

### Proposed future modeling sequence

```text
Stage 1: Clinical-only baseline
        ↓
Stage 2: Sensor-only features
        ↓
Stage 3: Clinical + sensor features
        ↓
Independent validation
        ↓
Prospective pilot evaluation
```

The central research question is whether sensor-derived functional measurements add useful information beyond conventional clinical inputs.

---

## 7. Reference measurements and validation

KneeMotion is not being developed in isolation from existing clinical measurement methods.

For development and analytical validation, the planned reference methods include:

- **Goniometer** for knee ROM
- **Stopwatch / video** for sit-to-stand timing
- Appropriate reference methods for walking/gait timing where available

The goal is not to replace these reference instruments during validation. The goal is to determine whether KneeMotion can produce sufficiently reliable functional measurements in a standardized, nurse-operated workflow and then add value through multi-parameter functional measurement and digital reporting.

---

## 8. Prototype roadmap

### Prototype 1 — Engineering POC

**Target:** ≤ ₹1,500 engineering target

Focus:

- Dual-IMU sensing
- Synchronization
- Calibration
- Placement repeatability
- Basic movement features

### Prototype 2 — Clinic-pilot target

**Target:** below ₹5,000 engineering target

Focus:

- Robust nurse-operated workflow
- Reusable clinic device
- Battery operation
- Tablet connectivity
- Standardized test protocol
- Repeatable session quality checks

### Prototype 3 — Enhanced / research configuration

**Target:** below ₹7,500 engineering target

Focus:

- Improved electronics / enclosure
- Better device health monitoring
- Research-grade sensing where justified
- Optional experimental channels only when they demonstrate incremental value

> Cost figures are engineering targets, not supplier quotations or manufacturing commitments.

---

## 9. Why the goniometer is still important

A goniometer remains a useful reference instrument for validating knee ROM.

KneeMotion is not justified merely because a goniometer exists or does not exist. The product case depends on whether the system can combine multiple functional measurements into a repeatable workflow that is easier to capture, digitize, review remotely and track longitudinally.

The intended value is therefore broader than ROM alone:

```text
Goniometer
→ knee-angle reference

KneeMotion
→ ROM + walking + sit-to-stand + movement timing
→ automated digital record
→ remote review
→ repeat measurements over time
```

The project should only scale if this additional information proves useful enough to justify the hardware, operating workflow and validation burden.

---

## 10. M-Swasth pilot concept

The proposed pilot is intentionally small and evidence-driven.

### Pilot workflow

```text
M-Swasth clinic
      ↓
Nurse fits sensors
      ↓
Guided standardized test
      ↓
Movement data capture
      ↓
Feature extraction + quality checks
      ↓
Structured report / risk-support signal
      ↓
Appropriate clinical pathway
      ↓
Follow-up / re-measurement
```

### Pilot questions

1. Does the hardware measure movement reliably?
2. Can different nurses operate the protocol consistently?
3. Can patients complete the test reliably?
4. Does the resulting information improve the care workflow?
5. Does sensor-derived information add value beyond basic clinical inputs?

### Proposed pilot measures

- Usable-data rate
- Test completion time
- Repeatability
- Nurse workload
- Patient acceptance
- Clinical usefulness
- Risk-model performance after appropriate independent validation

A controlled pilot should include a named clinical owner and an appropriate academic/clinical partner for independent validation and ethics oversight.

---

## 11. Broader platform vision

KneeMotion is the first application, not the entire platform.

Potential future applications include:

- **Balance / fall risk** — lower-back IMU and mobility protocols
- **Diabetic foot** — temperature sensing with optional pressure/motion
- **Shoulder / spine** — joint-specific motion assessment
- **ECG / PPG / SpO₂** — physiological monitoring
- **Respiratory monitoring** — spirometry or chest-band sensing
- **Fitness / performance** — movement and strength measurements

Each application requires its own protocol, evidence and validation.

The reusable foundation is the digital layer:

```text
Sensors
   +
Embedded acquisition
   +
Protocol engine
   +
Signal processing
   +
Quality control
   +
Data / reports
   +
Analytics
```

> **One platform. Many measurements. One healthcare network.**

---

## 12. Repository structure

```text
M-SWASTH_PROPOSAL_PROJECT/
│
├── Code_esp32.ino
│   └── Dual-MPU6050 raw acquisition / CSV logger
│
├── knee_oa_imu_logger.ino
│   └── Single-MPU6050 wiring and sensor test
│
├── Model_trial.py
│   └── Early XGBoost clinical research pipeline
│
├── knee_clinical_ml_ready.csv
│   └── Clinical dataset used for early model experimentation
│
└── README.md
    └── Project documentation
```

---

## 13. Running the firmware prototype

### Hardware

- ESP32 DevKit / compatible ESP32 board
- 2 × MPU6050
- Jumper wires / breadboard or prototype wiring
- USB cable

### Wiring

| Signal | ESP32 | MPU6050 Thigh | MPU6050 Shin |
|---|---:|---:|---:|
| SDA | GPIO 21 | SDA | SDA |
| SCL | GPIO 22 | SCL | SCL |
| VCC | 3.3 V | VCC | VCC |
| GND | GND | GND | GND |
| AD0 | — | GND → `0x68` | 3.3 V → `0x69` |

### Firmware

Open `Code_esp32.ino` in Arduino IDE or an equivalent ESP32-compatible workflow.

The firmware:

1. Starts I²C on GPIO 21/22
2. Scans for the two sensor addresses
3. Initializes both MPU6050s
4. Performs startup gyro-bias calibration
5. Captures approximately 100 Hz data
6. Emits timestamped CSV over USB serial

The current code intentionally stops at raw acquisition. Knee-angle estimation, gait analysis and clinical ML are separate downstream stages.

---

## 14. Running the ML prototype

Install the Python dependencies required by the script, including:

```bash
pip install pandas numpy scikit-learn xgboost
```

Update the local dataset/model paths in `Model_trial.py`, then run:

```bash
python Model_trial.py
```

This script is for **research development only**. Its current target construction and training setup are not suitable for clinical deployment.

---

## 15. Important limitations

This repository is a **research and engineering prototype**.

It is not currently:

- A clinically validated medical device
- A validated osteoarthritis diagnostic system
- A replacement for clinical examination
- A replacement for imaging when imaging is clinically indicated
- A validated autonomous treatment or prescription system

The hardware and analytics must undergo analytical validation, usability/reliability testing and appropriate clinical validation before deployment claims are made.

The current MPU6050-based prototype is appropriate for engineering development and bench work; a production design should use a properly qualified, current-generation IMU and production electronics.

---

## 16. Current next steps

1. Validate the dual-IMU acquisition and synchronization path.
2. Build reproducible sensor-to-segment calibration.
3. Validate knee ROM against a goniometer.
4. Validate sit-to-stand timing against stopwatch/video.
5. Validate short-walk measurements.
6. Design an independent clinical outcome and participant-level split strategy.
7. Test whether sensor data adds useful information beyond conventional clinical inputs.
8. Run a controlled M-Swasth pilot with appropriate clinical oversight.
9. Productize only the configuration that demonstrates measurable value.

---

## 17. Product principle

> **The device records. The software interprets. The care team decides.**

KneeMotion is being built to make objective functional measurement practical at the point of care — starting with one knee use case and expanding only where evidence supports the next application.

---

## 18. Project materials

The accompanying executive presentation presents the M-Swasth partnership proposition, the KneeMotion use case, the reusable IoT platform vision, prototype roadmap, validation approach and proposed pilot.

GitHub repository:

https://github.com/shiventaparia/M-SWASTH_PROPOSAL_PROJECT

---

## License and data notice

No explicit open-source software license is currently declared in the repository. Until a license is added, repository code should not be assumed to be freely reusable.

The clinical CSV included in the repository should also be treated carefully with respect to provenance, redistribution rights and licensing. Confirm the dataset source and permitted use before publishing or redistributing it as part of a public clinical project.
