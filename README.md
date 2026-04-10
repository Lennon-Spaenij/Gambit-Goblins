# Gambit-Goblins

A fully autonomous, 3D-printed chess-playing robot powered by an Arduino Uno Q. Created by the ingenious team: **the Gambit Goblins**.

---

## Overview

Gambit Goblins combines computer science with mechanical engineering to create a physical opponent for chess players. Using a SCARA robotic arm, the robot moves pieces across a standard chessboard based on moves calculated by the Stockfish chess engine — all driven by computer vision that watches the board through a camera.

The player makes their move physically on the board. The camera detects the move, passes it to the engine, and the robot arm physically plays the response.

---

## Features

- Fully autonomous chess gameplay against Stockfish
- Computer vision board detection (piece detection via OpenCV)
- SCARA arm with stepper motors and a servo gripper
- 20×4 I²C LCD interface with button navigation
- Per-player chess clock (30 min each)
- Selectable difficulty: Easy, Hard, Expert
- Stability detection — LCD shows "Please Wait" while the camera stabilizes after a robot move
- Resign option via long-press

---

## Technical Information

| Layer | Technology |
|---|---|
| Firmware | C++ (Arduino Uno Q) |
| Application logic | Python 3 |
| Development environment | Arduino App Lab (primary), Arduino IDE (early prototyping) |
| Chess engine | Stockfish (UCI) |
| Computer vision | OpenCV, NumPy |
| Chess logic | python-chess |
| Hardware communication | Arduino RouterBridge (Bridge API) |

---

## Hardware

- **Controller:** Arduino Uno Q
- **Arm type:** SCARA (3D-printed chassis)
- **Motors:** 3× stepper motors (X, Y, Z axes) via A4988/GRBL-style driver board
- **Gripper:** Servo motor
- **Display:** 20×4 I²C LCD (address 0x27)
- **Input:** 3 push buttons (Up, OK, Down)
- **Camera:** USB webcam (OpenCV-compatible)
- **Design tool:** Fritzing (wiring diagrams)

---
## Fritzing


<img width="1263" height="727" alt="Fritzing" src="https://github.com/user-attachments/assets/d899a39a-eb87-4559-b62a-e54b8c11becd" />

---

## Software Architecture

```
Arduino Uno Q (C++)              Python (Arduino App Lab)
┌─────────────────────┐          ┌───────────────────────────┐
│  Menu & UI logic    │          │  Camera calibration       │
│  Stepper control    │◄─Bridge─►│  Board state detection    │
│  Gripper control    │          │  Stockfish engine wrapper │
│  LCD display        │          │  Move detection (OpenCV)  │
│  Button handling    │          │  Reference frame tracking │
└─────────────────────┘          └───────────────────────────┘
```

The Arduino and Python sides communicate over the **Arduino RouterBridge** (`Bridge.call` / `Bridge.provide`), keeping hardware control on the Arduino side and all vision + chess logic on the Python side.

---

## Setup

1. **Calibrate** — Point the camera at the empty board and press OK to lock corner detection
2. **Verify** — Confirms the warped board image looks correct
3. **Initial Capture** — Place all pieces in starting position and press OK
4. **Difficulty** — Select Easy / Hard / Expert
5. **Start Game** — Robot plays White and makes the first move

---

## Scalability

The vision code is shaped in a way that it is compatible with different camera's and different types of robot arms.
You only need to adhere to the bridging logic or small tweaks are needed to suit your needs.

---

## Team

**The Gambit Goblins** : Engineered by the best goblins NHL-Stenden has.
