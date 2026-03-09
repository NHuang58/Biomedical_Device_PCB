# Biomedical Sensor Node for Keel Bone Monitoring

## Overview
This repository contains the hardware design files, firmware, and documentation for a custom, miniaturized Printed Circuit Board (PCB) designed to monitor keel bone dynamics in poultry. Keel bone damage is a significant welfare and economic issue in the commercial egg industry. This biomedical device aims to provide researchers with continuous, high-resolution, in-vivo kinetic data to better understand the biomechanical forces and environmental factors contributing to these injuries.

## Key Features
* **Miniaturized Footprint:** Designed with strict size and weight constraints to ensure the device does not alter the natural behavior of the animal.
* **Low-Power Architecture:** Utilizes an event-driven microcontroller and a wake-on-motion Inertial Measurement Unit (IMU) to maximize battery life during longitudinal studies.
* **Wireless Data Transfer:** Integrated Bluetooth Low Energy (BLE) module for seamless, wire-free data transmission to a central receiver in the housing facility.
* **Biocompatible Form Factor:** Hardware is optimized for eventual encapsulation in biocompatible epoxy or medical-grade silicone.

## Hardware Specifications
* **Microcontroller:** [Insert MCU Part Number, e.g., Nordic nRF52832]
* **Sensor:** [Insert IMU Part Number, e.g., TDK InvenSense ICM-20948 9-Axis MEMS]
* **Power Supply:** Coin cell battery (CR2032) or small Li-Po cell with onboard power management IC.
* **PCB Layers:** [Insert number, e.g., 4-layer] stack-up for high-density routing.

## Repository Structure
* `/Hardware`: Contains KiCad/Altium schematic files, PCB layout files, Gerber generation files, and the Bill of Materials (BOM).
* `/Firmware`: Contains the source code (C/C++) for the onboard microcontroller, including driver libraries for the IMU and BLE stack configurations.
* `/Documentation`: Datasheets for major components, system block diagrams, and mechanical CAD files for the device enclosure.
* `/Data_Analysis`: Python scripts for parsing, visualizing, and analyzing the telemetry data received from the sensor node.

## Getting Started

### Hardware
1. Clone this repository to your local machine.
2. Open the project file located in the `/Hardware` directory using your preferred ECAD software (e.g., KiCad, Altium Designer).
3. Review the schematic and PCB layout. The Gerber files required for manufacturing are located in `/Hardware/Gerbers`.

### Firmware
1. Navigate to the `/Firmware` directory.
2. Open the project in your IDE (e.g., VS Code, Segger Embedded Studio).
3. Ensure you have the appropriate SDK and toolchains installed for the selected microcontroller.
4. Compile the code and flash it to the board via the SWD debugging interface.

## License
This project is licensed under the [MIT License](LICENSE) - see the LICENSE file for details.