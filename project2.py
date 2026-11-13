import os
import csv
import serial
import matplotlib.pyplot as plt
import time
import numpy as np

baud = 9600
#portnum = 3 # no parity, display in ANSI
buffer_values = []
times = []
fin_times = []
intensity_values = []

try:
    UART_port = serial.Serial('COM3', baud, timeout = 1) # For windows users
except:
    UART_port = serial.Serial('/dev/cu.usbserial-0001', baud, timeout = 1) # For MacOS users

init_time = time.time() # time since the epoch in seconds
delta_time = init_time

try:
    if not UART_port.is_open: # make sure we can't cause a conflict
        UART_port.open()
        
    # when the time diff >= 60 we need the python to stop listening

    while delta_time - init_time <= 60:
        reading = (UART_port.read_until())
        buffer_values.append(reading)
        print(reading)
        delta_time = time.time()
        times.append(delta_time - init_time)

except:
     print("UART port disconnected! Please try again.")

finally:
    # don't need the port anymore, safer to close it right away.
    if UART_port.is_open: 
        UART_port.close()
    
buffer_values.pop(0) # the first value is probably not correcty formatted, safer to just discard
                     # it's possible to check if it needs to be but when there's hundreds of buffer_values,'
                     # losing one doesn't actually matter
                     

clean_buffer = []
clean_times = []

for i in range(len(buffer_values)):
    try:
        parts = buffer_values[i].strip()
        parts = parts.split(b',')
        if len(parts) != 2:
            continue
        adc_val = int(parts[0][2:9])
        intensity_val = int(parts[1][2:9])
        clean_buffer.append(adc_val)
        intensity_values.append(intensity_val)
        clean_times.append(times[i]) # only consider the matching time points, in case of data corruption
    except:
        continue

buffer_values = clean_buffer
fin_times = clean_times

plt.subplot(1, 2, 1)

# labelling for readability
plt.title("Raw ADC Reading over Time")
plt.xlabel("Time (s)")
plt.ylabel("Buffer Value")

plt.grid()
plt.plot(np.array(fin_times), np.array(buffer_values))


plt.subplot(1, 2, 2)
# labelling for readability
plt.title("Led Intesity over Time")
plt.xlabel("Time (s)")
plt.ylabel("Led Intensity (Duty Cycle %)")

# plot the intensity_values
plt.grid()
plt.plot(np.array(fin_times), np.array(intensity_values), color = 'r')


plt.show()

# now it's time to create a CSV file
# we always want to overwrite the previous one, just so there's only one file
head_names = ["Buffer Value", "Time (s)", "Intensity (Duty Cycle %)"] # header/field names
data = []

# now we create a list of dicts that the csv module can use
for i in range(len(buffer_values)):
    vals = {}
    vals["Buffer Value"] = buffer_values[i]
    vals["Time (s)"] = fin_times[i]
    vals["Intensity (Duty Cycle %)"] = intensity_values[i]
    data.append(vals)
    
new_csv = open("group_30.csv", 'w', newline='') # open the CSV file, write
csv_writer = csv.DictWriter(new_csv, fieldnames=head_names) # initialize a dictionary writer obj
csv_writer.writeheader() # write the header
csv_writer.writerows(data) # write the actual dataa
    
print(buffer_values)
