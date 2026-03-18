import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# 1. Load and Preprocess Data
PATH_NAME = "NiclaData.csv"
df = pd.read_csv(PATH_NAME)
df["timestamp"] = pd.to_datetime(df["timestamp"])

# Calculate Magnitude
df["accel_mag"] = np.sqrt(
    df["accelerometer_Ax"]**2 +
    df["accelerometer_Ay"]**2 +
    df["accelerometer_Az"]**2
)

# Rolling Window for Smoothing
df["accel_mag_smooth"] = df["accel_mag"].rolling(window=10, center=True).mean()

# 2. Determine Movement Threshold
threshold = df["accel_mag_smooth"].mean() + 2 * df["accel_mag_smooth"].std()
df["movement"] = df["accel_mag_smooth"] > threshold

# 3. Identify and Group Specific Events
# Create a unique ID for each contiguous block of movement
df["group"] = (df["movement"] != df["movement"].shift()).cumsum()
movement_groups = df[df["movement"]].groupby("group")

events = []
for group_id, group_data in movement_groups:
    peak_val = group_data["accel_mag_smooth"].max()
    peak_idx = group_data["accel_mag_smooth"].idxmax()
    events.append({
        "peak": peak_val,
        "index": peak_idx,
    })

# Convert to DataFrame and sort by peak magnitude to find small/med/large
events_df = pd.DataFrame(events).sort_values("peak").reset_index(drop=True)

# Select one event for each category (Smallest, Median/Middle, Largest)
# Note: This assumes at least 3 events were detected
small_event = events_df.iloc[0]
medium_event = events_df.iloc[len(events_df)//2]
large_event = events_df.iloc[-1]

# 4. Create the Comparison Plots
event_labels = ["Impact Magnitude (Soft)", "Impact Magnitude (Medium)", "Impact Magnitude (Hard)"]
event_data = [small_event, medium_event, large_event]

fig, axes = plt.subplots(1, 3, figsize=(18, 5), sharey=True)
window_size = 25  # Number of data points to show around the peak

for i, event in enumerate(event_data):
    center = int(event["index"])
    start = max(0, center - window_size)
    end = min(len(df), center + window_size)
    
    subset = df.iloc[start:end]
    axes[i].plot(subset["timestamp"], subset["accel_mag_smooth"], marker='o', label="Smoothed Accel")
    axes[i].axhline(threshold, color='red', linestyle='--', label="Threshold")
    axes[i].set_title(f"{event_labels[i]}\nPeak: {event['peak']:.2f}")
    axes[i].set_xlabel("Time")
    axes[i].tick_params(axis='x', rotation=45)

axes[0].set_ylabel("Acceleration Magnitude")
axes[0].legend()
plt.tight_layout()
plt.show()