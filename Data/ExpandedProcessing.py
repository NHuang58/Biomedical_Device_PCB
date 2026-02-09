import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def analyze_z_direction(file_path):
    try:
        df = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        return

    df['timestamp'] = pd.to_datetime(df['timestamp'])
    df = df.sort_values('timestamp').reset_index(drop=True)

    lsb_to_g = 1 / 4096.0
    g_to_ms2 = 9.80665
    
    df['Az_ms2'] = df['accelerometer_Az'] * lsb_to_g * g_to_ms2

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
    
    ax1.plot(df['timestamp'], df['Az_ms2'], color='#007acc', linewidth=1.0)
    ax1.set_ylabel('Acceleration Z ($m/s^2$)', fontsize=12)
    ax1.set_title('Z-Axis Acceleration', fontsize=14)
    ax1.grid(True, which='both', linestyle='--', alpha=0.7)

    ax2.plot(df['timestamp'], df['gyroscope_z'], color='#d8f41d', linewidth=1.0)
    ax2.set_ylabel('Gyroscope Z (LSB)', fontsize=12)
    ax2.set_title('Z-Axis Gyroscope', fontsize=14)
    ax2.grid(True, which='both', linestyle='--', alpha=0.7)

    ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M'))
    plt.xlabel('Time (UTC)', fontsize=12)
    plt.xticks(rotation=0)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    file_name = 'ExpandedAlarmData.csv' 
    analyze_z_direction(file_name)