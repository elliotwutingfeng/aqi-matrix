"""Fetch Air Quality Index (AQI) for a specified location and send it to an Arduino via serial communication."""

import json
import logging
import pathlib
import serial
import time
import tomllib
import urllib.request

from serial.tools import list_ports

logger = logging.getLogger(__name__)
logging.basicConfig(
    level=logging.INFO,
    format="%(levelname)s %(asctime)s - %(message)s",
    datefmt="%d %B %Y %H:%M:%S",
)


def main():
    try:
        with open(pathlib.Path(__file__).parent / "config.toml", "rb") as f:
            config = tomllib.load(f)
    except Exception as e:
        logger.error("Failed to load config.toml | %s", e)
        return

    AQICN_TOKEN = config.get("AQICN_TOKEN")
    LATITUDE = config.get("LATITUDE")
    LONGITUDE = config.get("LONGITUDE")

    if not (
        AQICN_TOKEN
        and isinstance(AQICN_TOKEN, str)
        and isinstance(LATITUDE, float)
        and -90 <= LATITUDE <= 90
        and isinstance(LONGITUDE, float)
        and -180 <= LONGITUDE <= 180
    ):
        logger.error(
            "Invalid config values for AQICN_TOKEN, LATITUDE, or LONGITUDE. Check config.toml."
        )
        return

    aqi_value = -2

    try:
        with urllib.request.urlopen(
            f"https://api.waqi.info/feed/geo:{LATITUDE};{LONGITUDE}/?token={AQICN_TOKEN}",
            timeout=30,
        ) as f:
            aqi = json.load(f).get("data", {}).get("aqi", None)
            aqi_value = int(aqi)
            if aqi_value < 0:
                raise ValueError("AQI value cannot be negative")
    except (json.JSONDecodeError, AttributeError) as e:
        logger.error("Failed to parse AQI data | %s", e)
        aqi_value = -3
    except TypeError as e:
        logger.error("AQI data is of wrong type | %s", e)
        aqi_value = -4
    except ValueError as e:
        logger.error("Invalid AQI data received | %s", e)
        aqi_value = -5
    except Exception as e:
        logger.error("Failed to fetch or process AQI data | %s", e)
        aqi_value = -2

    if aqi_value >= 0:
        logger.info("Current Air Quality Index (AQI): %d", aqi_value)

    if aqi_value == 0:
        aqi_value = -1  # Pyserial's auto-reset on connect sends 0 to Arduino. So for AQI 0, we send -1 instead to Arduino.

    try:
        port_device = next(
            (
                port.device
                for port in list_ports.comports()
                if "ACM" in port.device or "USB" in port.device
            ),
            None,
        )
        if not port_device:
            raise ConnectionError("No serial ports found.")
        with serial.Serial(
            port_device,
            baudrate=9600,
            timeout=5,
            write_timeout=5,
        ) as ser:
            time.sleep(3)  # Wait for serial connection to stabilize.
            ser.reset_output_buffer()
            ser.write(str(aqi_value).encode("utf-8") + b"\n")
            ser.flush()
        logger.info("AQI value sent to serial output.")

    except Exception as e:
        logger.error("Failed to send AQI data to Arduino | %s", e)


if __name__ == "__main__":
    main()
