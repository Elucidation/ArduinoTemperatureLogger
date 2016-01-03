import numpy as np
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from matplotlib.dates import DateFormatter
from datetime import datetime as dt
# Load data
print("Loading data...")
times = np.loadtxt('time.txt', dtype=int) # timestamps
N = times.shape[0] # Number of data points
temp_outside = np.loadtxt('t_out.txt', dtype=float) # indoor temperature
temp_inside = np.loadtxt('t_in.txt', dtype=float) # outdoor temperature

dates = np.zeros((N,1), dtype=dt)
for i in xrange(N):
  dates[i] = dt(2014, times[i,0], times[i,1], times[i,2], times[i,3])

print("Generating Plot...")
plt.plot(dates, temp_outside, 'b-', dates, temp_inside, 'r-');
plt.legend(["Outdoor", "Indoor"], loc=4);
plt.xlabel('Date Time (PST)');
plt.ylabel('Temperature (*C)');
plt.title('Apartment Indoor/Outdoor Temperatures');


formatter = DateFormatter('%m/%d %H:%M')
fig = plt.gcf()
fig.axes[0].xaxis.set_major_formatter(formatter)
# plt.show()

ax = plt = plt.gca()
ax.grid(True)

fig.set_size_inches(18.5,10.5)
fig.savefig('temperatures.png',dpi=100)
# plt.savefig('temperatures.png')
print("Finished.")

# # Generate dates
# dates = zeros(N,1);
# for i = 1:N
#     dates(i) = datenum([2014 times(i,:) 0]);
# end
# # Plot data
# plot(dates, temp_outside, 'b.-', dates, temp_inside, 'r.-');
# xpts = linspace(min(dates),max(dates),5);  %# Make a full vector, filling in missing dates
# set(gca,'XTick',xpts,'XTickLabel',datestr(xpts, 'mm/dd HH:MM'));  %# Set axes properties
# legend('Indoor', 'Outdoor');
# xlabel('Date Time (PST)');
# ylabel('Temperature (*C)');
# title('Apartment Indoor/Outdoor Temperatures');
