Home Monitoring System
---

Requirements
---
* Log indoor & outdoor temperatures every minute to SD card
* Timestamped log messages using RTC clock
* Display current indoor/outdoor temperature on LCD screen
* Ability to pop out SD card and get data without crashing Arduino

Desirements
---
* Display logged temperature in graph format





loop()
{
if minute has passed:
measureData();
logToSD();
}