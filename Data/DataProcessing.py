import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

PATH_NAME = "Movement1.csv"

df = pd.read_csv(PATH_NAME)

df["timestamp"] = pd.to_datetime(df["timestamp"])

df["accel_mag"] = np.sqrt(
    df["accelerometer_Ax"]**2 +
    df["accelerometer_Ay"]**2 +
    df["accelerometer_Az"]**2
)

# Rolling Window
df["accel_mag_smooth"] = df["accel_mag"].rolling(
    window=10, center=True
).mean()

# Determine the movement threshold with pandas
threshold = (
    df["accel_mag_smooth"].mean() +
    2 * df["accel_mag_smooth"].std()
)
df["movement"] = df["accel_mag_smooth"] > threshold

print(f"Movement threshold: {threshold:.2f}")

# Create plots
plt.figure()
plt.plot(df["timestamp"], df["accel_mag_smooth"], label="Smoothed Accel Magnitude")
plt.axhline(threshold, linestyle="--", label="Movement Threshold")
plt.xlabel("Time")
plt.ylabel("Acceleration Magnitude")
plt.title("Movement Detection")
plt.legend()
plt.show()

# Movement timestamps
movement_times = df.loc[df["movement"], "timestamp"]
print("Movement detected between:")
print(movement_times.min(), "and", movement_times.max())
