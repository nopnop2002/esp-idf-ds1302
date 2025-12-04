# esp-idf-ds1302
DS1302 RTC Driver for esp-idf

I ported from [here](https://github.com/Erriez/ErriezDS1302/tree/master/src).   

# Software requirements
ESP-IDF V5.0 or later.   
ESP-IDF V4.4 release branch reached EOL in July 2024.   
ESP-IDF V5.1 is required when using ESP32-C6.   

# Installation
```Shell
git clone https://github.com/nopnop2002/esp-idf-ds1302
cd esp-idf-ds1302
idf.py menuconfig
idf.py flash
```

# Wireing  

|DS1302||ESP32|ESP32Sn/Hn/Cn|
|:-:|:-:|:-:|:-:|
|CLK|--|GPIO15|GPIO1|
|DAT|--|GPIO16|GPIO2|
|RST|--|GPIO17|GPIO3|
|GND|--|GND|GND|
|VCC|--|3.3V|3.3V|

You can change GPIO using menuconfig.   


# Set Clock Mode   

This mode set RTC initial value using NTP time Server.   
You have to set these config value using menuconfig.   

![Image](https://github.com/user-attachments/assets/8f5f4924-b848-416c-8904-abf8d460fed2)
![Image](https://github.com/user-attachments/assets/123bccce-016d-4b9d-896f-988eca4e95db)

You have to set gpio & NTP Server using menuconfig.   

![Image](https://github.com/user-attachments/assets/ef1580f5-324c-485b-b006-233c574d79a9)
![Image](https://github.com/user-attachments/assets/6b013b47-3e96-4005-bd1c-4ea286a2638e)


# Get Clock Mode   

This mode take out the time from a RTC clock.   
You have to change mode using menuconfig.   

![Image](https://github.com/user-attachments/assets/ef1580f5-324c-485b-b006-233c574d79a9)
![Image](https://github.com/user-attachments/assets/31011055-3bf0-494d-b93f-19e73469d0b8)

![ds1302-13](https://user-images.githubusercontent.com/6020549/59513594-1fe96800-8ef6-11e9-9b53-a2d2626d784f.jpg)


# Get the time difference of NTP and RTC   

This mode get time over NTP, and take out the time from a RTC clock.   
Calculate time difference of NTP and RTC.   
You have to change mode using menuconfig.   

![Image](https://github.com/user-attachments/assets/ef1580f5-324c-485b-b006-233c574d79a9)
![Image](https://github.com/user-attachments/assets/a0278c3b-07ac-4817-bf55-95a429cd31f7)

![ds1302-14](https://user-images.githubusercontent.com/6020549/59556737-b2772d80-9002-11e9-921e-4a794605dd86.jpg)

# Time difference of 1 week later.   

![ds1302-1week](https://user-images.githubusercontent.com/6020549/59961747-e082d300-9516-11e9-87ea-dba01d00e3be.jpg)

# Time difference of 1 month later.   

![ds1302-1month](https://user-images.githubusercontent.com/6020549/61292629-b74f1d00-a80c-11e9-940a-51bd0aeef1d4.jpg)

# Comparison with other RTCs
This module has a large time lag.   
I recommend the DS3231 RTC.   
https://github.com/nopnop2002/esp-idf-ds3231
