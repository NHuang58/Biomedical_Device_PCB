import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def analyze_inertial_data_ieee(file_path):
    try:
        df = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        return

    # 1. Data Cleaning
    df['timestamp'] = pd.to_datetime(df['timestamp'])
    df = df.sort_values('timestamp').reset_index(drop=True)

    # 2. Conversion Factors
    # Accel: LSB to m/s^2 (assuming +/- 8g range: 4096 LSB/g)
    lsb_to_g = 1 / 4096.0
    g_to_ms2 = 9.80665
    
    # Gyro: LSB to Degrees Per Second (dps) 
    # Assuming +/- 2000 dps range: 16.4 LSB/dps
    lsb_to_dps = 1 / 16.4 
    
    # Process Accelerometer (Z-axis only as requested)
    df['Az_ms2'] = df['accelerometer_Az'] * lsb_to_g * g_to_ms2
    
    # Process Gyroscope (X, Y, Z)
    df['Gx_dps'] = df['gyroscope_x'] * lsb_to_dps
    df['Gy_dps'] = df['gyroscope_y'] * lsb_to_dps
    df['Gz_dps'] = df['gyroscope_z'] * lsb_to_dps

    # 3. IEEE Style Settings
    plt.rcParams['font.family'] = 'serif'
    plt.rcParams['font.serif'] = ['Times New Roman'] + plt.rcParams['font.serif']
    plt.rcParams['axes.labelweight'] = 'bold'

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8), sharex=True)
    
    # --- Top Plot: Z-Axis Accelerometer ---
    ax1.plot(df['timestamp'], df['Az_ms2'], color='black', linewidth=1.2, label='Accel Z')
    ax1.set_ylabel('Acceleration ($m/s^2$)', fontsize=10)
    ax1.set_title('Inertial Sensor Multi-Axis Analysis', fontsize=12)
    ax1.grid(True, linestyle=':', alpha=0.6)
    ax1.legend(loc='upper right', fontsize=9)

    # --- Bottom Plot: Tri-axial Gyroscope ---
    ax2.plot(df['timestamp'], df['Gx_dps'], color='#d9534f', linewidth=1.2, label='Gyro X', alpha=0.9)
    ax2.plot(df['timestamp'], df['Gy_dps'], color='#5bc0de', linewidth=1.2, label='Gyro Y', alpha=0.9)
    ax2.plot(df['timestamp'], df['Gz_dps'], color='#5cb85c', linewidth=1.2, label='Gyro Z', alpha=0.9)
    
    ax2.set_ylabel('Angular Velocity ($^{\circ}/s$)', fontsize=10)
    ax2.grid(True, linestyle=':', alpha=0.6)
    # Legend to distinguish the three axes
    ax2.legend(loc='upper right', fontsize=9, ncol=3) 

    # X-Axis Formatting
    ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    plt.xlabel('Time (HH:MM:SS)', fontsize=10)
    
    plt.tight_layout()
    
    # 4. Save for IEEE
    output_filename = 'Figure4.png'
    plt.savefig(output_filename, dpi=600, format='png', bbox_inches='tight')
    print(f"IEEE standard graph saved successfully: {output_filename}")
    
    plt.show()

if __name__ == "__main__":
    # Ensure CSV contains: timestamp, accelerometer_Az, gyroscope_x, gyroscope_y, gyroscope_z
    file_name = '1Hz_Test.csv' 
    analyze_inertial_data_ieee(file_name)