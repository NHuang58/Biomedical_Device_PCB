import asyncio
import csv
import datetime
from bleak import BleakClient, BleakScanner

GYRO_UUID = "19b10000-6001-537e-4f6c-d104768a1214"
TEMP_UUID = "19b10000-2001-537e-4f6c-d104768a1214"

output_file = f"nicla_data_{datetime.datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"
fieldnames = ["timestamp", "gyro_x", "gyro_y", "gyro_z", "temperature_C"]

gyro_data = [0.0, 0.0, 0.0]
temperature = 0.0

async def log_data():
    device = await BleakScanner.find_device_by_name("NiclaSenseME-Logger")
    if not device:
        print("Nicla device not found. Make sure it is advertising.")
        return

    async with BleakClient(device) as client:
        print(f"Connected to {device.name}")
        with open(output_file, mode="w", newline="") as file:
            writer = csv.DictWriter(file, fieldnames=fieldnames)
            writer.writeheader()

            def gyro_handler(sender, data):
                import struct
                gx, gy, gz = struct.unpack("fff", data)
                gyro_data[0], gyro_data[1], gyro_data[2] = gx, gy, gz

            def temp_handler(sender, data):
                import struct
                global temperature
                temperature = struct.unpack("f", data)[0]
                timestamp = datetime.datetime.now().isoformat()
                writer.writerow({
                    "timestamp": timestamp,
                    "gyro_x": gyro_data[0],
                    "gyro_y": gyro_data[1],
                    "gyro_z": gyro_data[2],
                    "temperature_C": temperature
                })
                file.flush()
                print(timestamp, gyro_data, temperature)

            await client.start_notify(GYRO_UUID, gyro_handler)
            await client.start_notify(TEMP_UUID, temp_handler)
            print("Logging data... Press Ctrl+C to stop.")
            while True:
                await asyncio.sleep(0.1)

asyncio.run(log_data())
