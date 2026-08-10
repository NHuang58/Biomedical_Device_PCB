# Nicla Sense ME Web-BLE Data Logger

Web Bluetooth dashboard for the Arduino Nicla Sense ME, extended for this project with
four custom biomedical sensors and a persistent, in-browser SQLite export.

Based on Arduino's official Nicla Sense ME BLE demo dashboard
(https://create.arduino.cc/editor/FT-CONTENT/333e2e07-ecc4-414c-bf08-005b611ddd75/preview).

## Monitored attributes

Stock Nicla Sense ME / BHY2 hub:
- Temperature, humidity, pressure
- Accelerometer, gyroscope, quaternion orientation
- Indoor air quality (BSEC), CO2, gas
- RGB LED control

Custom biomedical PCB sensors (added on `Database-Dev`):
- Heart rate (bpm)
- Internal temperature (°C) - onboard/enclosure temp, separate from the ambient sensor above
- Strain
- Force (N)

See [BLE_spec.txt](BLE_spec.txt) for the full characteristic/UUID list, and
[Arduino_Code/Sensor_Adjust_3.ino](../../Arduino_Code/Sensor_Adjust_3.ino) for the firmware.
The four custom sensors are currently wired up with placeholder analog-read
conversions (clearly marked `TODO` in the sketch) - swap in the real calibration once
the sensors are attached to the board.

## Running it

1. Flash `Arduino_Code/Sensor_Adjust_3.ino` to the Nicla Sense ME.
2. From this folder, start a local server (Web Bluetooth requires HTTP(S), not `file://`):
   ```
   python3 -m http.server 8080
   ```
3. Open `http://localhost:8080` in a Web Bluetooth-capable browser (Chrome/Edge on
   desktop or Android; not supported on iOS/Safari).
4. Click **CONNECT** and pick the `BLESense-XXXX` device advertised by the board.

## Data storage & export

Every incoming reading is written into an in-browser SQLite database (via
[sql.js](https://github.com/sql-js/sql.js), SQLite compiled to WebAssembly) as it
arrives, in addition to the small rolling buffer used to draw the live graphs. That
means the full session's data survives independent of the graph window, and is what
the export buttons pull from:

- **EXPORT DB** - downloads the entire session as a real `.sqlite` file
  (`NiclaData_<timestamp>.sqlite`), openable with any SQLite client (e.g. `sqlite3`,
  DB Browser for SQLite) or Python's built-in `sqlite3` module.
- **EXPORT CSV** - downloads the same data as a `.csv` file for quick inspection or
  use with the existing `Data/DataProcessing.py` / `FindPeaks.py` scripts.

The SQLite database only lives in the browser tab's memory - refreshing or closing the
page clears it, so export before you navigate away.

(c) 2022 Arduino SA, extended 2026 for this project.
License GPL
