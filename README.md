# UC1701

This library is used to control the onboard 128x64 LCD of Industruino. It allows you to display text and simple graphics with a minimum of overhead.

* [UC1701](#uc1701) - LCD display
* [U8G and U8G2](#u8g-and-u8g2) - LCD display

The Industruino LCD is connected over SPI to the pins D19,20,21,22 (and the backlight to D13 on 32u4 boards and D26 on 1286 boards). We suggest you use either of these 2 libraries:
* our customised UC1701 library (available in this repository): easy to use, relatively small (it is also used in the Industruino pre-installed demo sketches). it is largely compatible with the popular PCD8544 (Nokia screens). familiar syntax:  
```
lcd.begin();  
lcd.clear();  
lcd.setCursor(1, 1);  
lcd.print("hello Industruino!");
```
* U8G or U8G2 libraries

# U8G and U8G2
[U8G](https://github.com/olikraus/u8glib) is a popular display library with many fonts and graphics, consuming more memory than the basic UC1701 above. Use this constructor:
```
U8GLIB_MINI12864 u8g(21, 20, 19, 22);	// SPI Com: SCK = 21, MOSI = 20, CS = 19, A0 = 22
```

[U8G2](https://github.com/olikraus/u8g2) is the new improved version of the above U8G library, largely compatible, with 3 [buffer](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#buffer-size) options (speed vs memory). Use this constructor:
```
U8G2_UC1701_MINI12864_1_4W_SW_SPI u8g2(U8G2_R2, 21, 20, 19, 22);   // rotation, clock, data, cs, dc
```
