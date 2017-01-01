# Industruino libraries
*Please note that we only support the Arduino IDE up to version 1.6.5 at the moment.*

Arduino libraries to use with Industruino products:
* [UC1701](#uc1701) - LCD display
* [U8G and U8G2](#u8g-and-u8g2) - LCD display
* [EthernetIndustruino](#ethernetindustruino) - Ethernet module
* [Indio](#indio) - IND.I/O kit only
  * [digital I/O](#digital-io) 
  * [analog input](#analog-input)
  * [analog output](#analog-output)
  * [RS485](#rs485)
  * [interrupts](#interrupts)
  * [calibration](#calibration)

# UC1701
The Industruino LCD is connected over SPI to the pins D19,20,21,22 (and the backlight to D13 on 32u4 boards and D26 on 1286 boards). We suggest you use either of these 2 libraries:
* our customised UC1701 library (available in this repository): easy to use, relatively small (it is also used in the Industruino pre-installed demo sketches). it is largely compatible with the popular PCD8544 (Nokia screens). familiar syntax:  
```
lcd.begin();  
lcd.clear();  
lcd.setCursor(1, 1);  
lcd.print("hello Industruino!");
```

# U8G and U8G2
[U8G](https://github.com/olikraus/u8glib) is a popular display library with many fonts and graphics, consuming more memory than the basic UC1701 above. Use this constructor:
```
U8GLIB_MINI12864 u8g(21, 20, 19, 22);	// SPI Com: SCK = 21, MOSI = 20, CS = 19, A0 = 22
```

[U8G2](https://github.com/olikraus/u8g2) is the new improved version of the above U8G library, largely compatible, with 3 [buffer](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp#buffer-size) options (speed vs memory). Use this constructor:
```
U8G2_UC1701_MINI12864_1_4W_SW_SPI u8g2(U8G2_R2, 21, 20, 19, 22);   // rotation, clock, data, cs, dc
```

# EthernetIndustruino
If you are using the Industruino Ethernet module, you will need this library which is based on the standard Arduino Ethernet library. The Ethernet module is connected over SPI, so we also need the SPI library.
```
#include <SPI.h>
#include <EthernetIndustruino.h>
```
The Ethernet module also includes FRAM; see the example in the library on how to use this. If you want to use the FRAM together with the Ethernet, there is no need to include the SPI settings as in the FRAM example, because this is taken care of in the Ethernet library. So you can just include the 2 libraries with the above 2 lines; DO NOT include the SPI settings as in the FRAM example:
```
//Setting up the SPI bus -- NO NEED when using the EthernetIndustruino library
SPI.begin();
SPI.setDataMode(SPI_MODE0); 
SPI.setBitOrder(MSBFIRST);
SPI.setClockDivider(SPI_CLOCK_DIV2);
```

When using the Ethernet module with the Industruino PROTO, it is important to be aware of the I/O pins it is using, and which should not be used for other I/O functions:

| IDC pin number	| Module function	| Arduino pin	| Default connected	| Required for standard functions |
| --- | --- | --- | --- | --- |
| 1	|MISO	| D14	| yes	| yes |
| 2	| +5V	| +5V	| yes	| yes |
| 3	| SCLK	| D15/SCLK	| yes	| yes |
| 4	| MOSI	| D16/MOSI	| yes	| yes |
| 5	| Ethernet CS	| D10	| yes	| yes |
| 6	| GND	| GND	| yes	| yes |
| 7	| SD CS	| D4	| yes	| yes |
| 8	| Ethernet ext reset	| D5	| no	| no |
| 9	| Ethernet IRQ	| D7	| yes	| no |
| 10	| FRAM CS	| D6	| yes	| yes |
| 11	| /	| D0/RX	| no	| no |
| 12	| /	| D1/TX	| no	| no |
| 13	| /	| D2/SDA	| no	| no |
| 14	| /	| D3/SCL |	no	| no |


# Indio

##### Important notes:    
##### The digital and analog I/O will only work when Vin power (6.5-32V) is supplied to the Indio baseboard via the green screw connectors. When only USB power is connected, none of the digital or analog channels, nor the RS485, will work.
##### It is important to power down all systems (Industruino, sensors/actuators) before making connections to the Industruino.


If you are using the Industruino IND.I/O product, you will need this library to access the I/O channels. The pins on the IDC expansion connector, the backlight pin, and the membrane panel buttons pin(s) should still be accessed in the usual way, not using the Indio library; the Indio library is only for the external I/O channels available on the green screw connectors.

The Indio board uses an I2C expander for the I/O channels so we also need the Wire library.
```
#include <Indio.h>
#include <Wire.h>
```

### DIGITAL IO 

Configuration:
```
Indio.digitalMode(1,INPUT);       // Set CH1 as an input
Indio.digitalMode(7,OUTPUT);      // Set CH7 as an output
```
Read/write:
```
Indio.digitalRead(1);             // Read CH1
Indio.digitalWrite(7,LOW);        // Set CH7 to low (0V)
```

### ANALOG INPUT

##### Important note:    
##### The analog I/O section is galvanically isolated from the digital I/O section and the microcontroller section, to allow a separate power supply in the analog section for optimal accuracy. In case your analog sensors/actuators are on the same power supply as the digital section (Vin 12/24V) you have to connect the analog GND to the digital GND.

Configuration of resolution:
```
Indio.setADCResolution(16);       // Set the ADC resolution
                                  // Choices are 12bit@240SPS, 14bit@60SPS, 16bit@15SPS and 18bit@3.75SPS.
```

Configuration of input mode:
```
Indio.analogReadMode(1, V10);     // Set Analog-In CH1 to 10V mode (0-10V).
Indio.analogReadMode(1, V10_p);   // Set Analog-In CH1 to % 10V mode (0-10V -> 0-100%).
Indio.analogReadMode(1, V5);      // Set Analog-In CH1 to 5V mode (2x gain enabled on ADC).
Indio.analogReadMode(1, V5_p);    // Set Analog-In CH1 to 5V mode (0-5V -> 0-100%).
Indio.analogReadMode(1, V10_raw); // Set Analog-In CH1 to 10V mode and read raw ADC value (0-10V -> 0-4096).

Indio.analogReadMode(1, mA);      // Set Analog-In CH1 to mA mode (0-20mA).
Indio.analogReadMode(1, mA_p);    // Set Analog-In CH1 to % mA mode (4-20mA -> 0-100%)
Indio.analogReadMode(1, mA_raw);  // Set Analog-In CH1 to mA mode and read raw ADC value (0-20mA -> 0-4096).
```  
Read:
```
Indio.analogRead(1);              //Read Analog-In CH1 (output depending on selected mode as above)
```
Please note that the output of the Indio.analogRead() in RAW mode is not of the type INTEGER, but FLOAT. The output range is fixed from 0 to 4096 for all resolutions, but only in 12-bit mode this returns integers; for higher resolution the measurements are floating point numbers.

### ANALOG OUTPUT

##### Important note:   
#####The analog I/O section is galvanically isolated from the digital I/O section and the microcontroller section, to allow a separate power supply in the analog section for optimal accuracy. In case your analog sensors/actuators are on the same power supply as the digital section (Vin 12/24V) you have to connect the analog GND to the digital GND.

Configuration of output mode:
```
Indio.analogWriteMode(1, V10);      // Set Analog-Out CH1 to 10V mode (0-10V).
Indio.analogWriteMode(1, V10_p);    // Set Analog-Out CH1 to % 10V mode ( 0-100% -> 0-10V).
Indio.analogWriteMode(1, V10_raw);  // Set Analog-Out CH1 to 10V mode and take raw DAC value (0-4096 -> 0-10V).

Indio.analogWriteMode(1, mA);       // Set Analog-Out CH1 to mA mode (0-20mA).
Indio.analogWriteMode(1, mA_p);     // Set Analog-Out CH1 to % mA mode (0-100% -> 4-20mA).
Indio.analogWriteMode(1, mA_raw);   // Set Analog-Out CH1 to mA mode and take raw DAC value (0-4096 -> 0-20mA).   
```
Write (examples corresponding to above configuration):
```
Indio.analogWrite(1, 2.67, true);   //Set CH1 to 2.67V ("true" will write value to EEPROM of DAC, restoring it after power cycling).
Indio.analogWrite(1, 33.5, true);   //Set CH1 to 33.5% (approx 3.685V)
Indio.analogWrite(1, 1000, true);   //Set CH1 DAC to integer value 1000 (approx 2.685V).

Indio.analogWrite(1, 10.50, false); //Set CH1 to 10.5mA ("false" will not write value to EEPROM of DAC).
Indio.analogWrite(1, 75, true);     //Set CH1 to 75% (approx 16mA).
Indio.analogWrite(1, 2048, true);   //Set CH1 DAC to integer value 2048 (approx 10.5mA).
```

### INTERRUPTS

For using interrupts on the digital channels, the method depends on whether you're using topboard 32u4 or 1286. 

#### INTERRUPTS on the 32u4 topboard

The interrupt pin of the expander on the 12/24V digital side is connected to pin D8 (PCINT4) of the 32u4 topboard. This pin will trigger when a change on any of the 8 input or output channels occurs. If more than 1 channel needs to be detected by the interrupt, a flag can be set inside the interrupt service routine, and then any pin change can be checked inside the main loop, as discussed in this forum post https://industruino.com/forum/help-1/question/multiple-channels-interrupts-on-32u4-topboard-205

This code example (for 32u4 topboard) shows a counter on the LCD for each rising edge on CH1 (without debounce).
```
#include <Indio.h>
#include <Wire.h>

#include <UC1701.h>
static UC1701 lcd;

volatile int risingEdge = 1;
volatile int counter = 0;

void setup() {

  Serial.begin(9600);
  lcd.begin();
  Indio.digitalMode(1, OUTPUT); //  Clear CH1 to LOW
  Indio.digitalWrite(1, LOW); 
  Indio.digitalMode(1, INPUT); // Set CH1 as an input

  // Enable Pin Change Interrupt
  PCMSK0 = (1 << PCINT4);
  PCICR = (1 << PCIE0);

  // Global Interrupt Enable
  sei();
}

ISR (PCINT0_vect)
{
  if (risingEdge == 1) {
    risingEdge = 0;
    Serial.println("trigger");
    counter++;
  }

  else {
    risingEdge = 1;
  }
}

void loop() {
  lcd.setCursor(1,3);
  lcd.print(counter);
  delay(100);
}
```

#### INTERRUPTS on the 1286 topboard

The interrupt pin of the expander on the 12/24V digital side is connected to the INT7 pin of the 1286 topboard. This pin will trigger when a change on any of the 8 input or output channels occurs, and we can specify `CHANGE`, `RISING`, `FALLING`, `LOW` (note this pin is inverted: a change from LOW to HIGH on the digital channel triggers `FALLING`). If more than 1 channel needs to be detected by the interrupt, a flag can be set inside the interrupt service routine, and then any pin change can be checked inside the main loop, as discussed in this forum post https://industruino.com/forum/help-1/question/multiple-channels-interrupts-on-32u4-topboard-205

This code example (for 1286 topboard) shows a counter on the LCD for each rising edge on CH1 (without debounce).
```
#include <Indio.h>
#include <Wire.h>

#include <UC1701.h>
static UC1701 lcd;

volatile int counter = 0;

void setup() {

  Serial.begin(9600);
  lcd.begin();
  Indio.digitalMode(1, OUTPUT); //  Clear CH1 to LOW
  Indio.digitalWrite(1, LOW);  // 
  Indio.digitalMode(1, INPUT); // Set CH1 as an input

  attachInterrupt(7, count, CHANGE);       // INT7 attached to the interrupt of the expander
                                           // this is not D7
}

void count() {
  Serial.println("trigger");
  counter++;
}

void loop() {
  lcd.setCursor(1, 3);
  lcd.print(counter);
  delay(100);
}
```

The 1286 topboard also allows us to attach an interrupt to the membrane panel buttons: its button inputs are connected to pin change interrupts PCINT 4, 5, and 6 for buttons Down, Enter, and Up respectively.

Below demo sketch will show "waiting" on the LCD; when you press the "Enter" button an interrupt will be triggered and "Enter pressed" will show on the LCD for one second. To attach the interrupt to the "Up" or "Down" button change "PCINT5" in line `PCMSK0 = (1 << PCINT6);` to PCINT4 or PCINT6.

```
#include <UC1701.h>
static UC1701 lcd;

volatile int modeFlag = 0;

void setup() {
  
    lcd.begin(); //enable LCD
    // Enable Pin Change Interrupt 5 = Enter button
    PCMSK0 = (1 << PCINT5);
    PCICR = (1 << PCIE0);
 
    // Global Interrupt Enable
   sei();    
}

ISR (PCINT0_vect)
{    
    modeFlag = 1;    
}

void loop() { 
  
  lcd.setCursor(0, 0);
  lcd.print("waiting       ");
  
  if (modeFlag == 1) {
    lcd.setCursor(0, 0);
    lcd.print("Enter pressed");
    delay(1000); 
    modeFlag = 0;
  }
}
```

### RS485

RS485 is a popular industrial network standard and the INDIO features a half duplex RS485 transceiver. It is often used with the Modbus protocol, so-called Modbus RTU (as opposed to Modbus TCP which uses Ethernet). The Master unit sends out periodic requests over the network, and Slaves receive and reply.

Hardware specifics for RS485 on the INDIO:
* Serial connection = Serial1 (does not interfere with 'Serial', which can be used for uploading and Serial Monitor at the same time)
* TxEnablePin = D9

We can use the [SimpleModbusMaster and SimpleModbusSlave libraries](https://drive.google.com/folderview?id=0B0B286tJkafVYnBhNGo4N3poQ2c&usp=drive_web&tid=0B0B286tJkafVSENVcU1RQVBfSzg#list) (versions V2rev2 and V10 respectively) to establish communication over RS485 between 2 or more INDIOs, with one acting as the Master and the other one(s) as the Slave(s). This is one way of expanding the Industruino's number of I/O pins.

Basic configuration of the above Modbus RTU libraries:
```
#define baud       115200   // tested 9600 to 115200
#define timeout    1000
#define polling    20    // the scan rate
#define retry_count 10
// used to toggle the receive/transmit pin on the driver
#define TxEnablePin 9                                                           // INDUSTRUINO RS485

modbus_configure(&Serial1, baud, SERIAL_8N2, timeout, polling, retry_count, TxEnablePin, packets, TOTAL_NO_OF_PACKETS, regs);
```
For more information see the examples on our blog, e.g. [Modbus RTU between 2 INDIOs](https://industruino.com/blog/our-news-1/post/modbus-rtu-master-and-slave-14).

### CALIBRATION

Please find the calibration data array inside the Indio.cpp library file (as below), together with this explanation on how to perform the calibration. The library is preloaded with calibration data but characteristics are board specific thus reading with standard calibration data might be off. You are advised to update the calibration arrays in your own Indio.cpp file.

Analog Calibration method:
* Analog IN, 0-10V: In your sketch set the analogReadMode to V10_raw. Feed a known voltage between 0-10V into 1 of the 4 input channels. Read the corresponding raw ADC value from the serial terminal and write it down. Do this at two voltage levels, for example 2.5V and 7.5V. Enter the recorded raw ADC value and corresponding voltage (in mV) into the calibration array below. Repeat this for all 4 channels.
* Analog IN, 4-20mA: In your sketch set the analogReadMode to mA_raw. Feed a known current between 0-20mA into 1 of the 4 input channels. Read the corresponding raw ADC value from the serial terminal and write it down. Do this at two current levels, for example **10000uA and 20000uA**. Enter the recorded raw ADC value and corresponding voltage (in uA) into the calibration array below. Repeat this for all 4 channels. As a current source you can for example use a **500 Ohm** resistor in series with a 0-20V voltage source and multimeter in series.
* Analog OUT, 0-10V: In your sketch set the analogWriteMode to V10_raw. Set the analogWrite value to anything between 0-4096. Read the corresponding output voltage on a multimeter and write it down. Do this at two DAC output values, for example 1000 and 3600. Enter the recorded raw DAC value and corresponding voltage (in mV) into the calibration array below. Repeat this for all 2 channels.
* Analog OUT, **4-20mA**: In your sketch set the analogWriteMode to mA_raw. Set the analogWrite value to anything between 0-4096. Read the corresponding output current on a multimeter (connect multimeter between output and ground, best with a 100-500 Ohm resistor in series) and write it down. Do this at two DAC output values, for example 1000 and 3600. Enter the recorded raw DAC value and corresponding current (in uA) into the calibration array below. Repeat this for all 2 channels.

```
//Calibration data array for ADC, 0-10V mode

const int ADC_voltage_low_raw[5] = {0,384,384,382,386};//raw ADC value for low reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.
const int ADC_voltage_low_mV[5] = {0,1006,1006,1006,1006}; //corresponding mV for low reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.

const int ADC_voltage_high_raw[5] = {0,2939,2946,2952,2946}; //raw ADC value for high reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.
const int ADC_voltage_high_mV[5] = {0,7985,7985,7985,7985}; //corresponding mV for high reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.


//Calibration data array for ADC, 4-20mA mode

const int ADC_current_low_raw[5] = {0,1863,1863,1863,1863}; //raw ADC value for low reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.
const int ADC_current_low_uA[5] = {0,10000,10000,10000,10000}; //corresponding uA for low reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.

const int ADC_current_high_raw[5] = {0,3692,3692,3692,3692}; //raw ADC value for high reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.
const int ADC_current_high_uA[5] = {0,20000,20000,20000,20000}; //corresponding uA for high reference calibration point. Ignore first 0, subsequent is CH1-CH4 from left to right.


//Calibration data array for DAC, 0-10V mode

const int DAC_voltage_low_raw[3] = {0,1000,1000};//raw DAC value for low reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.
const int DAC_voltage_low_mV[3] = {0,2641,2630}; //corresponding mV for low reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.

const int DAC_voltage_high_raw[3] = {0,2500,2500}; //raw DAC value for high reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.
const int DAC_voltage_high_mV[3] = {0,6823,6805}; //corresponding mV for high reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.


//Calibration data array for DAC, 4-20mA mode

const int DAC_current_low_raw[3] = {0,1000,1000}; //raw DAC value for low reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.
const int DAC_current_low_uA[3] = {0,5162,5162}; //corresponding uA for low reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.

const int DAC_current_high_raw[3] = {0,3600,3600}; //raw DAC value for high reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.
const int DAC_current_high_uA[3] = {0,19530,19530}; //corresponding uA for high reference calibration point. Ignore first 0, subsequent is CH1-CH2 from left to right.
```


