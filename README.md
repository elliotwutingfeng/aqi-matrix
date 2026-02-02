# aqi-matrix

> "Haze, haze, go away."

[![LICENSE](https://img.shields.io/badge/LICENSE-BSD--3--CLAUSE-GREEN?style=for-the-badge)](LICENSE)

Air Quality Index (AQI) indicator using a WS2812B addressable LED matrix panel (NeoPixel-compatible).

Displays the current AQI value and changes color based on the current Air Quality Level at any specified geolocation using data from the [AQICN API](https://aqicn.org/api).

![aqi-matrix](aqi-matrix.png)

## You Will Need

- Tested on Raspberry Pi 1 Model B+ with ethernet. Raspberry Pi 3B or newer is recommended for built-in Wi-Fi.
- Power supply for Raspberry Pi (5V 2.5A recommended).
- MicroSD card (16 GB or larger recommended) for Raspberry Pi OS Lite (32-bit).
- 16 by 16 WS2812B addressable LED matrix panel.
- Arduino UNO R3 with USB cable.
- 3 jumper wires for connecting the LED matrix panel to the Arduino.
- 100 µF capacitor to prevent Arduino from resetting when a serial connection is opened.
- _Optional:_ 470 Ω resistor to prevent voltage spikes from damaging the LED matrix panel.
- _Optional:_ Enclosure for the Raspberry Pi, Arduino, and LED matrix panel; IKEA SANNAHED 25x25 cm works well, the inner 12x12 cm mount opening can be enlarged with a pen knife to fit the LED matrix panel snugly.

## Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software) on your computer.

1. From Arduino IDE, install the `Adafruit NeoMatrix` library, then open and upload [aqi-matrix.ino](aqi-matrix/aqi-matrix.ino) to your Arduino UNO R3.

1. Connect the WS2812B LED matrix panel to the Arduino as follows:
    - LED matrix panel **GND** to Arduino **GND**
    - LED matrix panel **5V** to Arduino **5V**
    - LED matrix panel **Data In** (⚠️ not **Data Out**) to Arduino **Digital Pin 6** (with optional 470 Ω resistor in series)

1. Place the 100 µF capacitor across **RESET** and **GND** on the Arduino (negative side to **GND**). This prevents the Arduino from resetting when a serial connection is opened.

1. Install Raspberry Pi OS Lite (32-bit) on your Raspberry Pi. You can download it from <https://www.raspberrypi.com/software/operating-systems/#raspberry-pi-os-32-bit-heading>. Once installed and logged in, upgrade the system packages and install the `python3-serial` package:

    ```bash
    sudo apt-get update && sudo apt-get upgrade -y && sudo apt-get install python3-serial && sudo apt-get autoremove -y
    ```

1. Copy `config-dev.toml` to `config.toml` and update the `AQICN_TOKEN`, `LATITUDE`, `LONGITUDE`, and `NUM_LEDS` in `config.toml`.

    ```bash
    cp --update=none ~/aqi-matrix/config-dev.toml ~/aqi-matrix/config.toml
    ```

    - Get your free `AQICN_TOKEN` by registering at <https://aqicn.org/data-platform/token>.
    - Find your current location's `LATITUDE` and `LONGITUDE` in decimal degrees using <https://gps-coordinates.net>.

1. Connect the Arduino to the Raspberry Pi via USB.

1. Run the script:

    ```bash
    python3 ~/aqi-matrix/aqi-matrix.py
    ```

    You should see a panda 🐼 on the LED matrix panel, with background color matching the AQI Air Quality Level,
    followed by the numerical AQI value.

## Scheduling

To have the script run automatically every minute, you can add it to your [crontab](https://en.wikipedia.org/wiki/Cron):

```bash
crontab -e
```

Add the following line:

```bash
*/1 * * * * python3 ~/aqi-matrix/aqi-matrix.py
```

Save and exit.

## Sources

- [AQICN API](https://aqicn.org/api) by AQICN
- [Piskel](https://www.piskelapp.com) Free and open-source online pixel art editor
- [Adafruit NeoPixel Uberguide](https://learn.adafruit.com/adafruit-neopixel-uberguide)
- [Adafruit NeoMatrix Library](https://github.com/adafruit/Adafruit_NeoMatrix)
