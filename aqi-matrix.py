"""Fetch Air Quality Index (AQI) for a specified location and send it to an Arduino via serial communication."""

import json
import logging
import math
import pathlib
import time
import urllib.request

import serial
import tomllib
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
    except Exception as e:  # noqa: BLE001
        logger.error("Failed to load config.toml | %s", e)
        return

    AQICN_TOKEN = config.get("AQICN_TOKEN")
    LATITUDE = config.get("LATITUDE")
    LONGITUDE = config.get("LONGITUDE")
    MAX_SEARCH_RADIUS_KM = config.get("MAX_SEARCH_RADIUS_KM")

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

    station_name = "UNKNOWN"
    aqi_value = -2
    search_radius_km = 10
    while search_radius_km < MAX_SEARCH_RADIUS_KM:
        # lat1 lng1 is northwest corner
        # lat2 lng2 is southeast corner

        # Approximate conversion from km to degrees
        lat1 = LATITUDE + search_radius_km / 111
        lng1 = LONGITUDE - search_radius_km / (
            111 * abs(math.cos(math.radians(LATITUDE)))
        )
        lat2 = LATITUDE - search_radius_km / 111
        lng2 = LONGITUDE + search_radius_km / (
            111 * abs(math.cos(math.radians(LATITUDE)))
        )

        try:
            with urllib.request.urlopen(
                f"https://api.waqi.info/map/bounds?token={AQICN_TOKEN}&networks=all&latlng={lat1},{lng1},{lat2},{lng2}",
                timeout=30,
            ) as f:
                # Get AQI value for the nearest valid station
                if (stations := json.load(f).get("data", [])) and isinstance(
                    stations, list
                ):
                    stations = [
                        s
                        for s in stations
                        if isinstance(s, dict)
                        and type(s.get("lat")) in (str, int, float)
                        and type(s.get("lon")) in (str, int, float)
                    ]
                    stations.sort(
                        key=lambda s: (
                            (float(s["lat"]) - LATITUDE) ** 2
                            + (float(s["lon"]) - LONGITUDE) ** 2
                        )  # Pythagorean distance approximation.
                    )
                if any(
                    (station := s)
                    for s in stations
                    if isinstance(s.get("aqi"), (str, int, float))
                    and int(s.get("aqi", -1)) >= 0
                ):
                    station_name = station.get("station", {}).get("name", "UNKNOWN")
                    aqi_value = int(station.get("aqi", -1))
                    break
        except Exception as e:  # noqa: BLE001
            logger.error(
                "Failed to fetch or process AQI data for search radius %d km | %s",
                search_radius_km,
                e,
            )
        search_radius_km += 10

    if aqi_value >= 0:
        logger.info(
            "Current Air Quality Index (AQI): %d | Station Name: %s",
            aqi_value,
            station_name,
        )

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

    except Exception as e:  # noqa: BLE001
        logger.error("Failed to send AQI data to Arduino | %s", e)


if __name__ == "__main__":
    main()
