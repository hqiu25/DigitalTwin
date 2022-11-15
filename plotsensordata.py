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
  def __init__(self, strPort, maxLen):
      # open serial port
      self.ser = serial.Serial(strPort, 250000)

      self.ax = deque([0.0]*maxLen)
      self.ay = deque([0.0]*maxLen)
      self.az = deque([0.0]*maxLen)
      self.av = deque([0.0]*maxLen)
      self.aw = deque([0.0]*maxLen)
      self.maxLen = maxLen

  # add to buffer
  def addToBuf(self, buf, val):
      if len(buf) < self.maxLen:
          buf.append(val)
      else:
          buf.pop()
          buf.appendleft(val)

  # add data
  def add(self, data):
      assert(len(data) == 7)
      self.addToBuf(self.ax, data[0])
      self.addToBuf(self.ay, data[1])
      self.addToBuf(self.az, (data[2]+data[3]+data[4])/3)
      self.addToBuf(self.av, data[5])
      self.addToBuf(self.aw, data[6])

  # update plot
  def update(self, frameNum, a0, a1, a2, a3, a4):
      try:
          line = self.ser.readline()
          data = [float(val) for val in line.split()]
          # Celsius:22.94,RPM:6,X:760,Y:865,Z:767
    
          # print data
          if(len(data) == 7):
              self.add(data)
              a0.set_data(range(self.maxLen), self.ax)
              a1.set_data(range(self.maxLen), self.ay)
              a2.set_data(range(self.maxLen), self.az)
              a3.set_data(range(self.maxLen), self.av)
              a4.set_data(range(self.maxLen), self.aw)
      except KeyboardInterrupt:
          print('exiting')
      
      return a0, 

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
  analogPlot = AnalogPlot(strPort, 100)

  print('plotting data...')

  # set up animation
#   fig = plt.figure()
#   ax = plt.axes(xlim=(0, 100), ylim=(0, 100))
#   a0, = ax.plot([], [])
#   a1, = ax.plot([], [])
#   a2, = ax.plot([], [])
# set up animation
  fig, (a0_axes_subplot, a1_axes_subplot, a2_axes_subplot, a3_axes_subplot, a4_axes_subplot) = plt.subplots(ncols=5, nrows=1)
  a0_axes_subplot.set_xlim(0, 100)
  a0_axes_subplot.set_ylim(20, 40)
  a1_axes_subplot.set_xlim(0, 100)
  a1_axes_subplot.set_ylim(0, 250)
  a2_axes_subplot.set_xlim(0, 100)
  a2_axes_subplot.set_ylim(400, 1000)
  a3_axes_subplot.set_xlim(0, 100)
  a3_axes_subplot.set_ylim(0, 1030)
  a4_axes_subplot.set_xlim(0, 100)
  a4_axes_subplot.set_ylim(700, 800)
  a0_axes_subplot.set_title('Temperature', fontfamily='serif', loc='left', fontsize='medium')
  a1_axes_subplot.set_title('Rotations Per Minute', fontfamily='serif', loc='left', fontsize='medium')
  a2_axes_subplot.set_title('Accelerometer', fontfamily='serif', loc='left', fontsize='medium')
  a3_axes_subplot.set_title('Voltage', fontfamily='serif', loc='left', fontsize='medium')
  a4_axes_subplot.set_title('Current', fontfamily='serif', loc='left', fontsize='medium')
  a0_line2d, = a0_axes_subplot.plot([], [])
  a1_line2d, = a1_axes_subplot.plot([], [])
  a2_line2d, = a2_axes_subplot.plot([], [])
  a3_line2d, = a3_axes_subplot.plot([], [])
  a4_line2d, = a4_axes_subplot.plot([], [])
  anim = animation.FuncAnimation(fig, analogPlot.update, 
                                  fargs=(a0_line2d, a1_line2d, a2_line2d, a3_line2d, a4_line2d), 
                                  interval=10)

  # show plot
  plt.show()
  
  # clean up
  analogPlot.close()

  print('exiting.')
  

# call main
if __name__ == '__main__':
  main()
