#!/bin/env python
# Takes daily data log text files from arduino yun and turns it into a CSV file with a header
# Also groups data by hour,day,week and month into separate aggregate csv files
import glob
from os.path import join
from datetime import datetime, timedelta
import numpy as np
import ntpath

# Location of Arduino Yun data logs relative to script
input_data_format = './raw_data/*.txt'
output_data_format = './raw_data/*.txt'
#ex. list for strings, ex. ['.\\R2015_06_13.txt',...]
input_file_list = glob.glob(input_data_format)

output_data_dir = './data/'

labels_all = ('datetime',
          'out_humidity', 'out_temperature',
          'in_dht11_humidity', 'in_dht11_temperature',
          'in_dht22_humidity', 'in_dht22_temperature' )


labels_hourly = ('datetime',
                'out_temperature_min',
                'out_temperature_mean',
                'out_temperature_max',
                )

hourly_filename = 'R2015_hourly.csv'
hourly_filepath = join(output_data_dir, hourly_filename)


daily_filename = 'R2015_daily.csv'
daily_filepath = join(output_data_dir, daily_filename)


# Format datestring from Arduino data log format to datetime object
def formatDate(datestring):
  return datetime.strptime(datestring,'%Y/%m/%d %H:%M')

data_hourly = np.zeros([24,3]) # min, mean, max
data_hourly_cnt = np.zeros(24)


with open(daily_filepath, 'w') as f_daily:
  f_daily.write("%s\n" % (",".join(labels_hourly))) # First line contains labels
  with open(hourly_filepath, 'w') as f_hourly:
    f_hourly.write("%s\n" % (",".join(labels_hourly))) # First line contains labels
    
    # For each file
    for input_filepath in input_file_list:
      # datetime object without hours/min
      input_filename = ntpath.basename(input_filepath)
      print ("Parsing %s..." % input_filename)
      file_date = datetime.strptime(input_filename,'R%Y_%m_%d.txt')

      # Init max/min so they get replaced
      data_hourly[:,0] = 100
      data_hourly[:,1] = 0
      data_hourly[:,2] = -100
      data_hourly_cnt[:] = 0

      # Writing minute data to daily csv
      csv_filename = '%s.csv' % input_filename[:-4]
      csv_filepath = join(output_data_dir, csv_filename)
      with open(csv_filepath, 'w') as f_csv:
        f_csv.write("%s\n" % (",".join(labels_all))) # First line contains labels
        # For each line in input file
        for line in open(input_filepath,'r'):
          linedata = line.strip().split('\t')
          timestamp = formatDate(linedata[0])
          linedata[0] = timestamp.isoformat()
          f_csv.write("%s\n" % (",".join(linedata)))

          # if not float(linedata[2]):
            
          temp = float(linedata[2])
          
          data_hourly[timestamp.hour,0] = min(temp, data_hourly[timestamp.hour,0])
          data_hourly[timestamp.hour,1] += temp
          data_hourly[timestamp.hour,2] = max(temp, data_hourly[timestamp.hour,2])
          data_hourly_cnt[timestamp.hour] += 1

        # Divide sum by counts to get mean
        data_hourly[data_hourly_cnt!=0,1] /= data_hourly_cnt[data_hourly_cnt!=0]
        data_hourly[data_hourly_cnt==0,1]=0

      print("Finished writing to %s" % csv_filename)

      # Write hourly data to file
      for hour in range(24):
        msg = "%s,%.2f,%.2f,%.2f\n" % (
          (file_date+timedelta(hours=hour)).isoformat(),
          data_hourly[hour, 0],
          data_hourly[hour, 1],
          data_hourly[hour, 2])
        f_hourly.write(msg)
      
      # Write daily message to file
      msg = "%s,%.2f,%.2f,%.2f\n" % (
        (file_date).isoformat(),
        data_hourly[:, 0].min(0),
        data_hourly[:, 1].mean(0),
        data_hourly[:, 2].max(0))
      f_daily.write(msg)
  print("Finished writing to %s" % hourly_filename)

print("Finished writing to %s" % daily_filename)