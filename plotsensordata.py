"""
ldr.py

Display analog data from Arduino using Python (matplotlib)

Author: Mahesh Venkitachalam
Website: electronut.in
"""

import sys, serial, argparse
import numpy as np
from time import sleep
from collections import deque

import matplotlib.pyplot as plt 
import matplotlib.animation as animation

    
# plot class
class AnalogPlot:
  # constr
  def __init__(self, strPort, maxLen, num_variables, plot_params):
      # open serial port
    self.ser = serial.Serial(strPort, 250000)
    self.plot_params = plot_params
    self.maxLen = maxLen
    self.num_variables = num_variables
    self.axes_values = []
    for i in range(len(plot_params)):
        self.axes_values.append(deque([0.0]*maxLen))

  # add to buffer
  def addToBuf(self, buf, val):
      if len(buf) < self.maxLen:
          buf.append(val)
      else:
          buf.pop()
          buf.appendleft(val)

  # add data
  def add(self, data):
    assert(len(data) == self.num_variables)
    self.addToBuf(self.axes_values[0], data[0])
    self.addToBuf(self.axes_values[1], data[1])
    self.addToBuf(self.axes_values[2], (data[2]+data[3]+data[4])/3)
    self.addToBuf(self.axes_values[3], data[5])
    self.addToBuf(self.axes_values[4], data[6])

  # update plot
  def update(self, *axes_line2d):
    axes_line2d = axes_line2d[1:]
    try:
        line = self.ser.readline()
        data = [float(val) for val in line.split()]
        if(len(data) == self.num_variables):
            self.add(data)
            for i in range(len(self.plot_params)):
                axes_line2d[i].set_data(range(self.maxLen), self.axes_values[i])
    except KeyboardInterrupt:
        print('exiting')
      
    return self.axes_values[0], 

  # clean up
  def close(self):
      # close serial
      self.ser.flush()
      self.ser.close()    

# main() function
def main():
  # create parser
  parser = argparse.ArgumentParser(description="LDR serial")
  # add expected arguments
  parser.add_argument('--port', dest='port', required=True)

  # parse args
  args = parser.parse_args()
  
  #strPort = '/dev/tty.usbserial-A7006Yqh'
  strPort = args.port

  print('reading from serial port %s...' % strPort)

  # plot parameters
  plot_params = [
    (0, 100, 20, 40, 'Temperature' ),
    (0, 100, 0, 250, 'Rotations Per Minute' ),
    (0, 100, 400, 1000, 'Accelerometer' ),
    (0, 100, 0, 1030, 'Voltage' ),
    (0, 100, 700, 800, 'Current' ),
  ]

  analogPlot = AnalogPlot(strPort, 100, 7, plot_params)

  print('plotting data...')

  # set up animation
  fig, axes_subplots = plt.subplots(ncols=len(plot_params), nrows=1)
  axes_line2d = []
  for i, (x_min, x_max, y_min, y_max, label) in enumerate(plot_params):
    axes_subplots[i].set_xlim(x_min, x_max)
    axes_subplots[i].set_ylim(y_min, y_max)
    axes_subplots[i].set_title(label, fontfamily='serif', loc='left', fontsize='medium')
    line2d, = axes_subplots[i].plot([], [])
    axes_line2d.append(line2d)
  anim = animation.FuncAnimation(fig, analogPlot.update, 
                                  fargs=axes_line2d, 
                                  interval=10)

  # show plot
  plt.show()
  
  # clean up
  analogPlot.close()

  print('exiting.')
  

# call main
if __name__ == '__main__':
  main()
