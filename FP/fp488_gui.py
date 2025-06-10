# pip install pyserial

import serial
import matplotlib.pyplot as plt
import re

# Define serial port and baud rate
COM_PORT = 'COM27'
BAUD_RATE = 115200

# Flags to indicate the type of data being processed
is_gui_cfg = False
is_peaks = False
# Flags for updating figure
plot_freqs = False
plot_ranges = False
# Holding figure objects
axvspan_objects = []
bar_objects = []

# Lists to store parsed data
channel_ranges = []
peaks = []

# Figure axis ranges
x_min = 0 
x_max = 2000
y_min = 0
y_max = 40

# Set up initial plot
plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots()
ax.set_facecolor('lightgray')
ax.grid(color='white', linestyle='--')
ax.set_xlabel('Frequency (Hz)', fontsize=14, fontweight='bold')
ax.set_ylabel('Magnitude', fontsize=14, fontweight='bold')
ax.set_title('Detected Peaks', fontsize=16, fontweight='bold')
ax.tick_params(axis='both', which='major', labelsize=12)
ax.set_xlim(x_min, x_max) 
ax.set_ylim(y_min, y_max) 
ax.grid(True)

# Open serial port
try:
    ser = serial.Serial(COM_PORT, BAUD_RATE)
    print(f"Serial port {COM_PORT} opened successfully.")
except serial.SerialException as e:
    print(f"Failed to open serial port {COM_PORT}: {e}")
    exit()

# Read and update plot continuously
while True:
    # Read data from serial port
    data = ser.readline().decode().strip()
    
    # Check if the data is in "gui cfg" format
    if data.startswith("gui cfg"):
        #print("Received gui cfg data.")
        is_gui_cfg = True
        is_peaks = False
        # Reset
        channel_ranges = []
        #ax.clear()
        
    # Check if the data is in "peaks" format
    elif data.startswith("peaks"):
        #print("Received peaks data.")
        is_peaks = True

    elif data.startswith("end"):
        if (is_gui_cfg):
            is_gui_cfg = False
            plot_ranges = True
        if(is_peaks):
            is_peaks = False
            plot_freqs = True

    # Parse the data based on flags
    elif is_gui_cfg:
        # Parse channel ranges
        match = re.match(r"\((\d+\.\d+), (\d+\.\d+)\)", data)
        if match:
            min_freq = float(match.group(1))
            max_freq = float(match.group(2))
            channel_ranges.append((min_freq, max_freq))
            #print(f"Parsed channel range: Min Frequency: {min_freq}, Max Frequency: {max_freq}")
    
    elif is_peaks:
        # Parse peaks data
        match = re.match(r"\((\d+\.\d+), (\d+\.\d+)\)", data)
        if match:
            frequency = float(match.group(1))
            magnitude = float(match.group(2))
            peaks.append((frequency, magnitude))
            #print(f"Parsed peak: Frequency: {frequency}, Magnitude: {magnitude}")
    
    # Update Plot
    if (plot_ranges or plot_freqs):
        # Reset plot flags
        plot_ranges = False
        plot_freqs = False
        
        # Plot shaded areas denoting channel ranges
        if plot_ranges:
            # Clear previous axvspan objects
            for axvspan_obj in axvspan_objects:
                axvspan_obj.remove()

            axvspan_objects = []  # Clear the list

            for min_freq, max_freq in channel_ranges:
                axvspan_obj = ax.axvspan(min_freq, max_freq, alpha=0.5, color='gray')
                axvspan_objects.append(axvspan_obj)
            
        # Plot peaks as vertical bars
        if plot_freqs:
            # Clear previous bar objects
            for bar_obj in bar_objects:
                bar_obj.remove()

            bar_objects = []  # Clear the list

            for frequency, magnitude in peaks:
                bar_obj = ax.bar(frequency, magnitude, color='salmon', width=10)
                bar_objects.append(bar_obj)
        
        peaks = []
        
        # Update the plot
        plt.draw()
        plt.pause(0.001)

