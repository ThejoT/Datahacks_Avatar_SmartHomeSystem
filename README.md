# Datahacks_Avatar_SmartHomeSystem
# FAYTe — Smart Home Environmental Monitoring System

**Winner, Mechanical Design & Biotechnology Track — DataHacks 2026 (UC San Diego)**

[Devpost writeup, photos, and demo video →](https://devpost.com/software/fayte)

A hardware environmental monitoring system for the home. FAYTe tracks temperature, humidity, soil moisture, and ambient light, then surfaces that data through a joystick-navigated LCD interface and an RGB status indicator — designed so a resident, including an elderly user, can read conditions at a glance without navigating an app.

![Model House](https://d112y698adiu2z.cloudfront.net/photos/production/software_photos/004/592/717/datas/gallery.jpg)

---

## What it does

| Measurement | Sensor | Output |
|---|---|---|
| Temperature & humidity | HS3003 (I2C) | LCD screen 1 |
| Soil moisture | Analog probe | LCD screen 2, mapped to 0–100% |
| Ambient light | Photoresistor | RGB LED — green (bright), blue (mid), red (dark) |
| Screen navigation | Analog joystick | Cycles between display screens |

A separate subsystem handles solar panel monitoring (`SolarPanelSystemCode`).

---

## Hardware

- Arduino board with `Arduino_HS300x` support (Nano 33 BLE Sense or equivalent)
- HS3003 temperature/humidity sensor (I2C, 100 kHz)
- 16x2 I2C LCD @ address `0x27`
- Analog soil moisture probe → `A0`
- Photoresistor → `A1`
- Analog joystick X-axis → `A2`
- RGB LED → pins `5` (R), `4` (G), `3` (B)

---

## Firmware design

Three implementation details worth noting:

**Non-blocking timing.** No `delay()` in the main loop. Display refresh (2 s) and sensor reconnect attempts (5 s) both run off `millis()` comparisons, so joystick input stays responsive while the system waits.

**Joystick edge detection.** A `joystickMoved` latch plus a center deadband (400–600) means one physical deflection advances exactly one screen. Without this, holding the stick would scroll continuously.

**I2C fault recovery.** If the HS3003 drops off the bus or returns an out-of-range reading (`isnan`, or outside −40 °C to 125 °C), the system flags the sensor offline, displays a reconnecting message, and every 5 seconds tears down and reinitializes the I2C bus to attempt recovery. The rest of the system keeps running throughout.

---

## Files

```
SmartSystemcode.ino      Main monitoring firmware — sensors, LCD, joystick, RGB, fault recovery
SolarPanelSystemCode     Solar panel subsystem firmware
```

---

## Build & flash

1. Install the Arduino IDE.
2. Install libraries: `LiquidCrystal_I2C`, `Arduino_HS300x`.
3. Wire per the pin map above. Confirm your LCD's I2C address — `0x27` is common but not universal.
4. Open `SmartSystemcode.ino`, select your board, and upload.
5. Open Serial Monitor at **115200 baud** for sensor logs.

If the LCD stays blank, scan for your LCD's actual I2C address and update the `LiquidCrystal_I2C` constructor.

---

## Roadmap

- Air quality, water leak, and motion sensing
- Wi-Fi connectivity with a companion app for remote monitoring by residents and caregivers
- Voice assistant that explains alerts in plain language for elderly users

---

## Team

Built at DataHacks 2026 by Thejo Tattala, Anjne Muralekrushna, Yovani Coyazo, and Forrest Olsen.

Enclosure and housing designed in Fusion 360, printed via Bambu Studio.
