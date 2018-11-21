# -*- coding: utf-8 -*-
"""
Created on Wed Sep 12 14:27:25 2018

@author: TIONG
"""

import time
import serial
import numpy as np
import pyqtgraph as pg
import math
import sys

from pyqtgraph.Qt import QtGui

delay = time.sleep(3) # buffer time

#%% Function

def getValue():
    """
    Read sensor output: mq3
    Return an array of [mq3]
    """
    readSensor = ((ser.readline().strip()).decode('ascii')).split(',')
    sensorVal = np.zeros(len(readSensor))
    for i in range(len(readSensor)):
        try:
            sensorVal[i]=float(readSensor[i])
        except ValueError:
            sensorVal[i]=0.0
    return sensorVal


def graphValues(time, val):
    """
    Plot mq3 graph in real-time

    val contains [mq3]
    """

    valmq3 = val[:].copy()

    curve_mq3.setData(time, valmq3)
    app.processEvents()
    return

def writeToFile(times, mq3, filename):
    """
    Save data to CSV file
    """
    file = open(filename, 'w')
    file.write("Time (S)"+ "," + "MQ3 (V)" + "\n")

    for i in range(0, len(times)):
        file.write(str(times[i]) + "," + str(mq3[i]) + "\n")
    file.close()

#%% Initialize parameters

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Adjustable parameters
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
total_time = 120.0*0.5 # time in seconds
xaxis_time = 10.0 #number of seconds shown in x-axis at a time
moving_avg_range = 5 # moving window size
serial_port = 'COM9'

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Define parameter
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
RESOLUTION = 1024.0 # max value from the sensor
MAX_VOLTAGE = 5 # max voltage from the sensor
SAMPLING_TIME = 0.1 # amount of time between each sample in seconds
TOTAL_DATA = total_time/SAMPLING_TIME
XAXIS_RANGE = int(xaxis_time/SAMPLING_TIME)


'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Serial Communication
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
ser = serial.Serial(serial_port, 9600, timeout=40)
ser.flushInput()

'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Graph
'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
app = QtGui.QApplication(sys.argv)
pg.setConfigOption('background', 'w')
pg.setConfigOption('foreground', 'k')
window = pg.GraphicsWindow()
window.resize(1000,600)
window.setWindowTitle("MQ3 Sensor")
window.show()

window_1 = window.addPlot(title='MQ3')   # Graph 1
window_1.setLabels(left=('Voltage (V)'))
window_1.setLabels(bottom=('Time (s)'))
window_1.addLegend()

curve_mq3 = window_1.plot(pen=(0,255,0), name="MQ3")   # Initialize plot



#%% Main Program

start = time.time() # start to measure execution time
n_data = 0 # number of raw signal
n_filt = 0 # number of filtered signal

rawVal = [] # raw signal
rawTime = [] # timestamp of the raw signal

filtVal = [] # filtered signal
filtTime = [] # timestamp of the filtered signal

times = np.zeros(0)
mq3 = np.zeros(0)
filtData = np.zeros(0)
filtDatatime = np.zeros(0)

while n_data<=TOTAL_DATA: # start program

    sensorRead = getValue() # read signal from sensor [X, Y, Z, PD]

    sensorRead = sensorRead * MAX_VOLTAGE/RESOLUTION # convert red PD signal into voltage
    rawTime_temp = n_data*SAMPLING_TIME # timestamp

    times = np.append(times, rawTime_temp) # total running time
    mq3 = np.append(mq3, np.array([sensorRead[0]]))


    if n_data<moving_avg_range: # collecting raw signal for moving average filter
        rawVal = np.append(rawVal, sensorRead)
        rawTime = np.append(rawTime, rawTime_temp)

    else:

        filtVal_temp = np.mean(rawVal, axis=0)
        filtTime_temp = rawTime[math.floor(moving_avg_range/2)]
        filtData = np.append(filtData, filtVal_temp)
        filtDatatime = np.append(filtDatatime, filtTime_temp)


        if n_filt<XAXIS_RANGE: # collecting filtered signal
            filtVal = np.append(filtVal, filtVal_temp)
            filtTime = np.append(filtTime, filtTime_temp)
            n_filt+=1

        lr = len(rawTime)

        rawVal[0:-1] = rawVal[1:lr]
        rawTime[0:-1] = rawTime[1:lr]

        rawVal[lr-1] = sensorRead
        rawTime[lr-1] = rawTime_temp


    if n_data>=XAXIS_RANGE: # Plot
        graphValues(filtTime, filtVal)

        lf = len(filtTime)

        filtVal[0:-1] = filtVal[1:lf]
        filtTime[0:-1] = filtTime[1:lf]

        filtVal[lf-1] = filtVal_temp
        filtTime[lf-1] = filtTime_temp

    else:
        if n_filt>=1:
            graphValues(filtTime, filtVal)

    n_data+=1

writeToFile(times, mq3, "rawData.csv")
writeToFile(filtDatatime, filtData[:], "filteredData.csv")

end = time.time()
print("Data Acquisition Time : ", end - start) # print program run timekkk5

ser.close()
#window.close()


