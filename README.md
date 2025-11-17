# 🕒 Arduino Word Clock (Wortuhr) – DS1307 LED Time Display

An Arduino-powered **word clock** that displays the current time using illuminated German words instead of digits.
Hours, minute phrases, minute-dots, and even a 5-second progress bar are rendered through dedicated LED pins.
Designed for reliability, clarity, and hardware flexibility.

---

## ✨ Features

* **DS1307 RTC integration** (`Wire/I2C`) with validity check
* **Automatic RTC reset to compile time** if the stored time is invalid
* **12-hour display** with hour-transition rule:

  * *minutes < 25 → current hour*
  * *minutes ≥ 25 → next hour*
* **Minute phrases** (FÜNF, ZEHN, VIERTEL, ZWANZIG, VOR, NACH, HALB)
* **Minute remainder LEDs** for 1–4 minutes
* **Second progress bar** (pins 2..13) in 5-second steps
* **Daylight-saving switch** (hardware toggle on pin 51)
* **Poti-based fine adjustment** (±15 minutes)
* **Startup self-test**: two LED sweep sequences
* Robust LED reset that only affects actual output pins

---

## 🛠️ Hardware Requirements

* **Arduino (Mega recommended)** – needs many GPIO pins
* **RTC DS1307**
* **LED matrix or discrete LEDs** mapped to pins 2–13 and 22–50
* **DST switch** on **pin 51** (with *external pull-down*)
* **Optional**: potentiometer on **A8** for minute offset

---

## 📌 Pin Mapping (Overview)

### Minute words (22–29)

FÜNF, ZEHN, ZWANZIG, DREI, VIERTEL, VOR, NACH, HALB

### Hour words (mapped via array)

EINS, ZWEI, DREI, VIER, FÜNF, SECHS, SIEBEN, ACHT, NEUN, ZEHN, ELF, ZWÖLF

### Minute dots (44–47)

1–4 minutes

### Seconds bar (2–13)

5-second increments, full bar at 59s

---

## ⚙️ Configuration

| Setting          | Description                       |
| ---------------- | --------------------------------- |
| `delayy`         | Animation and test timing         |
| `buttonpin` (51) | DST toggle: HIGH = summer time    |
| `potipin` (A8)   | Minute fine-offset (-15 to +15)   |
| `hourpins` table | Maps hour words to their LED pins |

---

## 🚦 Self-Test on Boot

1. LED sweep on **22–50**
2. LED sweep on **2–13**
3. RTC validation and optional auto-reset

---

## 🧪 Known Issues

* DS1307 may drift over time → periodic resync recommended
* Hardware tolerances may require small fine-offset adjustments (via Poti)

---

## 📅 Changelog (excerpt)

**27.08.2025**

* Fixed “ELF”, “ZEHN”, and minute-dot mappings
* Clean LED reset logic
* DST switch enabled by default
* Improved robustness in hour handling

---

## 📘 Future Improvements

* Poti smoothing + configurable offset range
* Switch debouncing
* Optional DS3231 upgrade (better accuracy)
* Night mode / PWM brightness control

## 🛠️ BIG TODO:

* rework all over
* split in separate files
* refactor

---

## 📜 License

MIT – free to use, modify, and build upon.



