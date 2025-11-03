import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.widgets import Slider, TextBox

# File to load
filename = "nicla_data_raw.csv"

# Load data
data = pd.read_csv(filename)
data["timestamp"] = pd.to_datetime(data["timestamp"])
data = data.sort_values("timestamp").reset_index(drop=True)
data["elapsed_s"] = (data["timestamp"] - data["timestamp"].iloc[0]).dt.total_seconds()

t_min = data["elapsed_s"].iloc[0]
t_max = data["elapsed_s"].iloc[-1]

# Colors
colors = {
    "gx": "#00FFFF", "gy": "#7FFF00", "gz": "#FF6FFF",
    "ax": "#1E90FF", "ay": "#32CD32", "az": "#FF4500",
    "temp": "#FFD700"
}

plt.style.use("dark_background")
fig, axs = plt.subplots(7, 1, figsize=(12, 10), sharex=True)
fig.patch.set_facecolor("black")
plt.subplots_adjust(left=0.1, right=0.95, top=0.94, bottom=0.2, hspace=0.3)

# Gyroscope subplots
axs[0].plot(data["elapsed_s"], data["gyroscope_x"], color=colors["gx"], linewidth=1)
axs[0].set_ylabel("Gyro X (°/s)", color=colors["gx"])
axs[1].plot(data["elapsed_s"], data["gyroscope_y"], color=colors["gy"], linewidth=1)
axs[1].set_ylabel("Gyro Y (°/s)", color=colors["gy"])
axs[2].plot(data["elapsed_s"], data["gyroscope_z"], color=colors["gz"], linewidth=1)
axs[2].set_ylabel("Gyro Z (°/s)", color=colors["gz"])

# Accelerometer subplots
axs[3].plot(data["elapsed_s"], data["accelerometer_Ax"], color=colors["ax"], linewidth=1)
axs[3].set_ylabel("Acc X (m/s²)", color=colors["ax"])
axs[4].plot(data["elapsed_s"], data["accelerometer_Ay"], color=colors["ay"], linewidth=1)
axs[4].set_ylabel("Acc Y (m/s²)", color=colors["ay"])
axs[5].plot(data["elapsed_s"], data["accelerometer_Az"], color=colors["az"], linewidth=1)
axs[5].set_ylabel("Acc Z (m/s²)", color=colors["az"])

# Temperature subplot
axs[6].plot(data["elapsed_s"], data["temperature_temperature"], color=colors["temp"], linewidth=1.5)
axs[6].set_ylabel("Temp (°C)", color=colors["temp"])
axs[6].set_xlabel("Time (s)")

# Style adjustments
for ax in axs:
    ax.grid(True, color="#333333", linestyle="--", linewidth=0.5)
    ax.tick_params(colors="white")
    for spine in ax.spines.values():
        spine.set_color("white")
    ax.yaxis.label.set_color("white")

# Slider for time navigation
slider_ax = plt.axes([0.1, 0.08, 0.65, 0.03], facecolor="#222222")
slider = Slider(slider_ax, "Time Center (s)", valmin=t_min, valmax=t_max, valinit=t_max, color="#888888")

# Text box for changing window size
textbox_ax = plt.axes([0.8, 0.08, 0.12, 0.04], facecolor="#111111")
textbox = TextBox(textbox_ax, "", initial="30", color="#222222", hovercolor="#444444")
textbox.text_disp.set_color("white")
textbox.label.set_color("white")

# Default time window
WINDOW = 30.0

def update_plot(center):
    global WINDOW
    half = WINDOW / 2.0
    start = center - half
    end = center + half

    if start < t_min:
        start = t_min
        end = min(t_min + WINDOW, t_max)
    if end > t_max:
        end = t_max
        start = max(t_min, t_max - WINDOW)

    for ax in axs:
        ax.set_xlim(start, end)
    fig.canvas.draw_idle()

def slider_update(val):
    update_plot(float(val))

def textbox_update(text):
    global WINDOW
    try:
        WINDOW = float(text)
        if WINDOW <= 0:
            WINDOW = 1.0
        update_plot(slider.val)
    except ValueError:
        pass  # ignore invalid inputs

slider.on_changed(slider_update)
textbox.on_submit(textbox_update)

# Initialize
update_plot(t_max)
plt.suptitle("Nicla Sense ME Data Viewer", color="white", fontsize=14)
plt.show()
