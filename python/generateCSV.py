#!/bin/env python
# Takes daily data log text files from arduino yun and turns it into a CSV file with a header
# Also groups data by hour,day,week and month into separate aggregate csv files
import glob
from os.path import join
import numpy as np
from datetime import datetime

# Location of Arduino Yun data logs relative to script
data_format = './raw_data/*.txt'
#ex. list for strings, ex. ['.\\R2015_06_13.txt',...]
print(glob.glob(data_format))


labels = ('datetime',
          'out_humidity', 'out_temperature',
          'in_dht11_humidity', 'in_dht11_temperature',
          'in_dht22_humidity', 'in_dht22_temperature' )


raw_filedir = './raw_data'
filename = 'R2015_06_14.txt'

raw_filepath = join(raw_filedir, filename)
data_filedir = './data'

csv_filename = "%s.csv" % filename[:-4]
csv_filepath = join(data_filedir, csv_filename)

data = []

# Format datestring from Arduino data log format to datetime object
def formatDate(datestring):
  return datetime.strptime(datestring,'%Y/%m/%d %H:%M')


data_hourly = np.zeros([24,3]) # min, mean, max
# Init max/min so they get replaced
data_hourly[:,0] = 100
data_hourly[:,2] = -100
data_hourly_cnt = np.zeros(24)

with open(csv_filepath, 'w') as f:
  f.write("%s\n" % (",".join(labels))) # First line contains labels
  for line in open(raw_filepath,'r'):
    linedata = line.strip().split('\t')
    timestamp = formatDate(linedata[0])
    linedata[0] = timestamp.isoformat()
    f.write("%s\n" % (",".join(linedata)))

    temp = float(linedata[2])
    
    data_hourly[timestamp.hour,0] = min(temp, data_hourly[timestamp.hour,0])
    data_hourly[timestamp.hour,1] += temp
    data_hourly[timestamp.hour,2] = max(temp, data_hourly[timestamp.hour,2])
    data_hourly_cnt[timestamp.hour] += 1

# Divide sum by counts to get mean
data_hourly[:,1] /= data_hourly_cnt

print(data_hourly)