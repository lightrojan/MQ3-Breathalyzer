# MQ3-Breathalyzer

Simple Blood Alcohol Concentration (BAC) project.

Hardware Used:

1) Arduino Nano
2) ESP32
3) MQ3 Alcohol Sensor
4) SparkFun LiPo Charger/Booster - 5V/1A
5) 0.91'' 128X32 I2C White Oled Display Diy Module Dc3.3V 5V

Database Used:

AWS EC2
phpmyadmin
mySQL

Operating Instructions:

1) Installing esp32 in Arduino IDE follow tutorial: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/ 
2) Upload "esp32.ino" to esp32 by choosing "esp DEV module". Upload "nano_bac_serial_wifi_bt_push.ino" to arduino nano.
3) Device waits for wifi connection to be established (Wifi credentials can be changed in "esp32.ino") before going into selection screen for wifi(hold push button for initialised duration) or bluetooth mode(quick push). 
5) Collection of BAC levels can then be done via blowing into MQ3 sensor.
6) For wifi mode, collection of data will be done over a set period and peak BAC value displayed on screen before resetting.

For wifi mode:

1) Upload POST script "mysql_connect.php" to database host. Change servername,username etc to the corresponding credentials of database.
2) May use a FTP client like filezilla to upload the file into "var/www/html" folder of web host.
3) Ensure that The POST 'URL' to the location of your insert script "xxx.php" on your web-host is changed in "esp32.ino" file.

For BT mode:
1) Ensure "ESP32" is paired with computer. 
2) **TO NOTE: Initial pairing to device can be done after pushing button to start BT mode.

Power:
1) LiPo battery can be charged using USB **USB charging via computer might not be strong enough, recommended to charge through wall outlet.
