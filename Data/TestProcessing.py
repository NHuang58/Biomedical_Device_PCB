import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

def analyze_nicla_data(file_path):
    # 1. Load the data
    try:
        df = pd.read_csv(file_path)
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")
        return

    df['timestamp'] = pd.to_datetime(df['timestamp'])
    
    # Sort data chronologically (CSV is often reverse chronological)
    df = df.sort_values('timestamp').reset_index(drop=True)

    # 3. Unit Conversion
    # Accelerometer: 1g is approx 4096 LSB (based on Z-axis ~4100 at rest)
    # 1g = 9.80665 m/s^2
    lsb_to_g = 1 / 4096.0
    g_to_ms2 = 9.80665
    
    # Calculate components in m/s^2
    df['Ax_ms2'] = df['accelerometer_Ax'] * lsb_to_g * g_to_ms2
    df['Ay_ms2'] = df['accelerometer_Ay'] * lsb_to_g * g_to_ms2
    df['Az_ms2'] = df['accelerometer_Az'] * lsb_to_g * g_to_ms2
    
    # Calculate Magnitude (Total Acceleration)
    df['Acc_Mag'] = np.sqrt(df['Ax_ms2']**2 + df['Ay_ms2']**2 + df['Az_ms2']**2)

    # Gyroscope: Keeping as Raw LSB (Least Significant Bits) as scale factor is unknown
    # Calculate Magnitude (Total Rotation)
    df['Gyro_Mag'] = np.sqrt(df['gyroscope_x']**2 + df['gyroscope_y']**2 + df['gyroscope_z']**2)

    # 4. Identify Perturbations at the end (Last 10 seconds)
    last_timestamp = df['timestamp'].iloc[-1]
    start_of_end = last_timestamp - pd.Timedelta(seconds=10)
    
    end_data = df[df['timestamp'] >= start_of_end]
    # Find peaks in the end region that are significantly above noise
    peaks = end_data[end_data['Gyro_Mag'] > 20]

    print("-" * 50)
    print(f"ANALYSIS REPORT: {file_path}")
    print("-" * 50)
    print(f"Total Duration: {df['timestamp'].iloc[0].time()} to {df['timestamp'].iloc[-1].time()}")
    print("\nDetected Perturbations in the last 10 seconds:")
    if not peaks.empty:
        print(peaks[['timestamp', 'Gyro_Mag', 'Acc_Mag']].to_string(index=False))
    else:
        print("No significant perturbations detected in the last 10 seconds.")
    print("-" * 50)

    # 5. Plotting
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
    
    # Plot Accelerometer
    ax1.plot(df['timestamp'], df['Acc_Mag'], color='#007acc', label='Total Acceleration', linewidth=1.5)
    ax1.set_ylabel('Acceleration ($m/s^2$)', fontsize=12)
    ax1.set_title('Accelerometer Magnitude (Full Timeline)', fontsize=14)
    ax1.grid(True, which='both', linestyle='--', alpha=0.7)
    ax1.legend(loc='upper left')

    # Plot Gyroscope
    ax2.plot(df['timestamp'], df['Gyro_Mag'], color='#d8f41d', label='Gyro Magnitude', linewidth=1.5)
    # Overlay raw data as shadow to show noise floor
    ax2.fill_between(df['timestamp'], df['Gyro_Mag'], 0, color='#d8f41d', alpha=0.3)
    
    # Annotate the specific peaks found at the end
    for idx, row in peaks.iterrows():
        ax2.annotate(f"{row['Gyro_Mag']:.0f}", 
                     xy=(row['timestamp'], row['Gyro_Mag']), 
                     xytext=(0, 10), textcoords='offset points',
                     ha='center', color='black', fontsize=9, fontweight='bold',
                     arrowprops=dict(arrowstyle='->', color='black'))

    ax2.set_ylabel('Gyroscope (Raw LSB)', fontsize=12)
    ax2.set_title('Gyroscope Magnitude (Highlighting Perturbations)', fontsize=14)
    ax2.grid(True, which='both', linestyle='--', alpha=0.7)
    ax2.legend(loc='upper left')

    # Format Time Axis
    ax2.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))
    plt.xlabel('Time (UTC)', fontsize=12)
    plt.xticks(rotation=0)
    
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Ensure the filename matches your uploaded file
    file_name = 'AlarmData.csv' 
    analyze_nicla_data(file_name)