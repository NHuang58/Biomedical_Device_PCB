# Operational Manual: Biomedical Sensor Node for Keel Bone Monitoring

This document serves as the standard operating procedure and technical manual for deploying, operating, and managing data from the custom miniaturized Printed Circuit Board (PCB) designed to monitor keel bone dynamics in poultry. 

---

## 1. Hardware Specifications and Setup

### 1.1 Physical Components
The biomedical sensor node consists of a custom four-layer PCB integrated into an animal-wearable form factor. Key components include:
* **Microcontroller:** Nordic Semiconductor nRF52840 SoC (supporting ARM Cortex-M4F and Bluetooth Low Energy 5.0 protocols).
* **Inertial Measurement Unit (IMU):** TDK InvenSense ICM-20948 9-axis MEMS sensor (3-axis gyroscope, 3-axis accelerometer, and 3-axis compass).
* **Power Supply:** Onboard rechargeable Lithium-Polymer (Li-Po) battery circuit regulated by a Microchip MCP73831 charge management controller.
* **Storage:** External SPI flash memory for high-fidelity data logging when wireless telemetry is obstructed.

### 1.2 Encapsulation and Biocompatibility
To protect the electronics from moisture, bodily fluids, and mechanical stress, the hardware must be encapsulated before deployment:
1. Ensure the battery is securely attached and the board has been programmed and post-assembly tested.
2. Verify that the total package weight does not exceed the target threshold for the specific avian subject to avoid altering natural gait and behavior.

### 1.3 Power and Charging Control
* **Charging Interface:** Connect a standard micro-USB or USB-C cable (depending on the revision variant) to the onboard charging port.
* **LED Status Indicators:**
  * **Solid Red:** Battery is actively charging.
  * **Solid Green / Off:** Charging cycle complete.
* **Power Toggle:** The node features a low-profile slide switch or magnetic reed switch to cycle between active deployment mode and deep sleep storage mode.

---

## 2. System Operation

### 2.1 Firmware Flashing and Initialization
Before deploying a new node, the device firmware must be flashed via Serial Wire Debug (SWD):
1. Connect the hardware node to an external programmer (e.g., J-Link) using the SWD test points on the PCB.
2. Open the source code repository under the `/Firmware` directory in Segger Embedded Studio or VS Code with the nRF Connect SDK.
3. Build the solution and flash the binary image onto the nRF52840.
4. On initial power-up, the device runs a power-on self-test (POST) to verify communication with the ICM-20948 IMU over the SPI interface and validates flash memory allocation.

### 2.2 Bluetooth Low Energy (BLE) Connectivity
The sensor node operates as a BLE peripheral using an event-driven architecture to maximize battery life:
1. **Advertising State:** Upon activation, the device begins advertising its unique UUID (configured in the firmware configuration header).
2. **Connection Establishment:** Use a central receiver unit, a central gateway node, or a mobile testing app (such as nRF Connect) to pair with the device.
3. **Wake-on-Motion (WoM):** To minimize power, the device enters a low-power sleep mode during periods of animal inactivity. It wakes automatically and resumes active high-frequency transmissions when acceleration forces exceed the pre-configured hardware threshold.

---

## 3. Data Collection and Telemetry Protocol

### 3.1 Sensor Configuration
The system collects 9-axis motion data mapped to the anatomical axes of the subject:
* **Accelerometer Range:** Configurable up to ±16g to capture high-impact loading forces during landings or collisions.
* **Gyroscope Range:** Configurable up to ±2000 dps to capture rotational kinetic dynamics during flight or wing flapping.
* **Sampling Rate:** Default configuration is 100 Hz for high-resolution analysis, adjustable via downstream BLE commands.

### 3.2 Data Telemetry and Local Storage
* **Real-Time Telemetry:** When connected to a central base station, data packets containing timestamps, 3D acceleration, and 3D angular velocity are streamed continuously via custom BLE GATT characteristics.
* **Onboard Logging:** If the connection to the gateway is lost or if the study protocol demands local logging, data packets are serialized and written directly to the external flash memory.
* **Memory Management:** A circular buffer strategy prevents total data loss. When storage capacity is filled, oldest records are overwritten unless the firmware parameter `STOP_ON_FULL` is enabled.

### 3.3 Data Parsing and Downstream Analysis
To extract and process the binary telemetry streams:
1. Navigate to the `/Data_Analysis` directory in the repository.
2. Execute the provided Python extraction scripts to parse the raw byte blocks into structured comma-separated values (CSV) format.
3. Use the analysis libraries to calculate total impact magnitudes, isolate peak acceleration events, and filter high-frequency noise from the raw kinetic data streams.# Operational Manual: Biomedical Sensor Node for Keel Bone Monitoring

This document serves as the standard operating procedure and technical manual for deploying, operating, and managing data from the custom miniaturized Printed Circuit Board (PCB) designed to monitor keel bone dynamics in poultry. 

---

## 1. Hardware Specifications and Setup

### 1.1 Physical Components
The biomedical sensor node consists of a custom four-layer PCB integrated into an animal-wearable form factor. Key components include:
* **Microcontroller:** Nordic Semiconductor nRF52840 SoC (supporting ARM Cortex-M4F and Bluetooth Low Energy 5.0 protocols).
* **Inertial Measurement Unit (IMU):** TDK InvenSense ICM-20948 9-axis MEMS sensor (3-axis gyroscope, 3-axis accelerometer, and 3-axis compass).
* **Power Supply:** Onboard rechargeable Lithium-Polymer (Li-Po) battery circuit regulated by a Microchip MCP73831 charge management controller.
* **Storage:** External SPI flash memory for high-fidelity data logging when wireless telemetry is obstructed.

### 1.2 Encapsulation and Biocompatibility
To protect the electronics from moisture, bodily fluids, and mechanical stress, the hardware must be encapsulated before deployment:
1. Ensure the battery is securely attached and the board has been programmed and post-assembly tested.
2. Clean the assembly with isopropyl alcohol to remove solder flux residue.
3. Pot the assembly in medical-grade silicone or biocompatible epoxy (such as Smooth-On Dragon Skin or Loctite M-21HP).
4. Verify that the total package weight does not exceed the target threshold for the specific avian subject to avoid altering natural gait and behavior.

### 1.3 Power and Charging Control
* **Charging Interface:** Connect a standard micro-USB or USB-C cable (depending on the revision variant) to the onboard charging port.
* **LED Status Indicators:**
  * **Solid Red:** Battery is actively charging.
  * **Solid Green / Off:** Charging cycle complete.
* **Power Toggle:** The node features a low-profile slide switch or magnetic reed switch to cycle between active deployment mode and deep sleep storage mode.

---

## 2. System Operation

### 2.1 Firmware Flashing and Initialization
Before deploying a new node, the device firmware must be flashed via Serial Wire Debug (SWD):
1. Connect the hardware node to an external programmer (e.g., J-Link) using the SWD test points on the PCB.
2. Open the source code repository under the `/Firmware` directory in Segger Embedded Studio or VS Code with the nRF Connect SDK.
3. Build the solution and flash the binary image onto the nRF52840.
4. On initial power-up, the device runs a power-on self-test (POST) to verify communication with the ICM-20948 IMU over the SPI interface and validates flash memory allocation.

### 2.2 Bluetooth Low Energy (BLE) Connectivity
The sensor node operates as a BLE peripheral using an event-driven architecture to maximize battery life:
1. **Advertising State:** Upon activation, the device begins advertising its unique UUID (configured in the firmware configuration header).
2. **Connection Establishment:** Use a central receiver unit, a central gateway node, or a mobile testing app (such as nRF Connect) to pair with the device.
3. **Wake-on-Motion (WoM):** To minimize power, the device enters a low-power sleep mode during periods of animal inactivity. It wakes automatically and resumes active high-frequency transmissions when acceleration forces exceed the pre-configured hardware threshold.

---

## 3. Data Collection and Telemetry Protocol

### 3.1 Sensor Configuration
The system collects 9-axis motion data mapped to the anatomical axes of the subject:
* **Accelerometer Range:** Configurable up to ±16g to capture high-impact loading forces during landings or collisions.
* **Gyroscope Range:** Configurable up to ±2000 dps to capture rotational kinetic dynamics during flight or wing flapping.
* **Sampling Rate:** Default configuration is 100 Hz for high-resolution analysis, adjustable via downstream BLE commands.

### 3.2 Data Telemetry and Local Storage
* **Real-Time Telemetry:** When connected to a central base station, data packets containing timestamps, 3D acceleration, and 3D angular velocity are streamed continuously via custom BLE GATT characteristics.
* **Onboard Logging:** If the connection to the gateway is lost or if the study protocol demands local logging, data packets are serialized and written directly to the external flash memory.
* **Memory Management:** A circular buffer strategy prevents total data loss. When storage capacity is filled, oldest records are overwritten unless the firmware parameter `STOP_ON_FULL` is enabled.

### 3.3 Data Parsing and Downstream Analysis
To extract and process the binary telemetry streams:
1. Navigate to the `/Data_Analysis` directory in the repository.
2. Execute the provided Python extraction scripts to parse the raw byte blocks into structured comma-separated values (CSV) format.
3. Use the analysis libraries to calculate total impact magnitudes, isolate peak acceleration events, and filter high-frequency noise from the raw kinetic data streams.