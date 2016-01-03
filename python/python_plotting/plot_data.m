%% Load data
times = load('time.txt'); % timestamps
N = size(times,1); % Number of data points
temp_outside = load('t_out.txt'); % indoor temperature
temp_inside = load('t_in.txt'); % outdoor temperature

%% Generate dates
dates = zeros(N,1);
for i = 1:N
    dates(i) = datenum([2014 times(i,:) 0]);
end

%% Plot data
plot(dates, temp_outside, 'b.-', dates, temp_inside, 'r.-');

xpts = linspace(min(dates),max(dates),5);  %# Make a full vector, filling in missing dates
set(gca,'XTick',xpts,'XTickLabel',datestr(xpts, 'mm/dd HH:MM'));  %# Set axes properties
legend('Indoor', 'Outdoor');
xlabel('Date Time (PST)');
ylabel('Temperature (*C)');
title('Apartment Indoor/Outdoor Temperatures');