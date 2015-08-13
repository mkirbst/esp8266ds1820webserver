# esp8266ds1820webserver
Arduino version of ESP8266 DS1820 webserver using DHCP

- this version uses just the arduino ide, this worked flawless for me, as long as is used +5V VCC when i programmed the ESP8266
- no need for lua
- code consists of snippeds found by google (contact me if you see your code and dont want that it show up here)
- look at the *.ino file for wiring scheme and adjust your correct wlan settings (wlan ssid and wlan password)

HTML/text output looks like: 

[sensor address] [sensor type] [temperature value in celsius] [crc checksum]

e.g.:

28-FF-F9-EC-50-14-00-EA DS18B20 25.87 b7

(can easily parsed with wget and awk or your favorite scripting tools, i use a munin plugin to do this)



