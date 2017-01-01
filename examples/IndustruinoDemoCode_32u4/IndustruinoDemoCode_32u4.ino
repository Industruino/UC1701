/*
* Industruino Demo Code - Default code loaded onto Industruino REV3
*
* Copyright (c) 2013 Loic De Buck <contact.industruino@gmail.com>
*
* Industruino is a DIN-rail mountable Arduino Leonardo compatible product
* Please visit www.industruino.com for further information and code examples.
* Standard peripherals connected to Industruino are:
* UC1701 compatible LCD; rst:D19 dc:D20 dn:D21 sclk:D22 (set pin configuration in UC1701 library header) 
* 3-button membrane panel; analog pin A5
*/


#include <UC1701.h>
//Download libary from https://github.com/Industruino/

// A custom glyph (a smiley)...
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };


static UC1701 lcd;

//menu defines

//- initial cursor parameters
int coll = 0; //column counter for cursor - always kept at 0 in this demo (left side of the screen)
int channel = 0; //Counter is controlled by the up&down buttons on the membrane panel. Has double use; 1. As row controller for the cursor (screen displays 6 rows of text, counting from 0 to 5). 2. As editor for numerical values shown on screen 
int lastChannel = 0; //keeps track of previous 'channel'. Is used to detect change in state.

//- initial menu level parameters
int MenuLevel = 0; //Defines the depth of the menu tree
int MenuID = 0; //Defines the unique identifier of each menu that resides on the same menu level
int channelUpLimit = 5; //Defines the upper limit of the button counter: 1. To limit cursor's downward row movement 2. To set the upper limit of value that is beeing edited.
int channelLowLimit = 0; //Defines the lower limit of the button counter: 1. To limit cursor's upward row movement 2. To set the lower limit of value that is beeing edited.

//- initial parameters for 'value editing mode'
int valueEditing = 0; //Flag to indicate if the interface is in 'value editing mode', thus disabling cursor movement.
int row = 0; //Temporary location to store the current cursor position whilst in 'value editing mode'.
int constrainEnc = 1; //Enable/disable constraining the button panel's counter to a lower and upper limit.
float valueEditingInc = 0; //Increments of each button press when using 'value editing mode'. 
float TargetValue = 0; // Target value to be edited in 'value editing mode'

//Membrane panel button defines

int buttonUpState = 0; //status of "Up" button input
int buttonEnterState = 0; //status of "Enter" button input
int buttonDownState = 0; //status of "Down" button input

int prevBtnUp = 0; //previous state of "Up" button
int prevBtnEnt = 0; //previous state of "Enter" button
int prevBtnDown = 0; //previous state of "Down" button

int lastBtnUp = 0; //time since last "Up" pressed event
int lastBtnEnt = 0; //time since last "Enter" pressed event
int lastBtnDown = 0; //time since last "Down" pressed event

int enterPressed = 0; //status of "Enter" button after debounce filtering : 1 = pressed 0 = unpressed

int transEntInt = 250; //debounce treshold for "Enter" button
int transInt = 100; //debounce for other buttons
unsigned long lastAdminActionTime = 0; //keeps track of last button activity

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A5;  // Analog input pin that the button panel is attached to
const int backlightPin = 13; // PWM output pin that the LED backlight is attached to
const int D0 = 0;
const int D1 = 1;
const int D2 = 2;
const int D3 = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
const int D8 = 8;
const int D9 = 9;
const int D10 = 10;
const int D11 = 11;
const int D12 = 12;
const int D14 = 14;
const int D15 = 15;
const int D16 = 16;
const int D17 = 17;


int ButtonsAnalogValue = 0;        // value read from mebrane panel buttons.
int backlightIntensity = 0;        // LCD backlight intesity
int backlightIntensityDef = 0;     // Default LCD backlight intesity
unsigned long lastLCDredraw = 0;   // keeps track of last time the screen was redrawn

void setup() {
  
  SetInput(); //Sets all general pins to input
  backlightIntensity = EEPROM.read(0); //loads the backlight intensity from EEPROM
  pinMode(backlightPin, OUTPUT); //set backlight pin to output 
  analogWrite(backlightPin, (map(backlightIntensity, 0, 5, 255, 0))); //convert backlight intesity from a value of 0-5 to a value of 0-255 for PWM.

//LCD init
  lcd.begin();  //sets the resolution of the LCD screen
  
  for (int y=0; y <= 7; y++){
for (int x=0; x <= 128; x++){
        lcd.setCursor(x, y);
        lcd.print(" ");
   } 
}
  
//debug  
 Serial.begin(9600); //enables Serial port for debugging messages

//Menu init  
 MenuSplash(); //load first menu

}

/*
* 1. The loop function calls a function to check the buttons (this could also be driven by timer interrupt) and updates the button counter (variable called 'channel'), which increases when 'Down' button is pressed and decreases when "Up" buttons is pressed.
* 2. Next, the loop function calls the 'Navigate' function which draws the cursor in a position based on the button counter, and when the "Enter" button is pressed checks which new menu should be  loaded or what other action to perform.
* 3. Each menu's content and scope is defined in a separate function. Each menu should have a defined 'MenuLevel' (depth of the menu tree, starting from 0) and unique MenuID so that the Navigate function can discern which menu is active.

*To make your own menus you should take 2 steps:

*1. make a new menu function, edit the parameters such MenuLevel and MenuID, scope of the cursor (number of rows, constraints etc).
*2. Edit the 'Navigate' function to reflect the menu function that you just made and assigning an action to it. 
*/

void loop() { 
  
  ReadButtons(); //check buttons
  Navigate(); //update menus and perform actions
  delay(50);
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//UI menu content - edit, add or remove these functions to make your own menu structure
//These functions only generate the content that is printed to the screen, please also edit the "Navigate" function further below to add actions to each menu.
//------------------------------------------------------------------------------------------------------------------------------------------------------------

void MenuSplash() { //this function draws the first menu - splash screen
//menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 0; //upper row limit
  channelLowLimit = 0; //lower row limit
  MenuLevel = 0; //menu tree depth -> first level
  MenuID = 0; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
//actual user content on the screen
  lcd.setCursor(5, 2); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Industruino"); //print text on screen
}

void MenuMain() { //second menu - choice of submenu's
//menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 2; //upper row limit
  MenuLevel = 1; //menu tree depth -> second level
  MenuID = 1; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
  ScrollCursor(); //enable the moving cursor (note that this function is not called in the splash screen, thus disabling the cursor)
//actual user content on the screen
  lcd.setCursor(6, 0); //set the cursor to the sixth pixel from the left edge, first row.
  lcd.print("Setup"); //print text on screen
  lcd.setCursor(6, 1); //set the cursor to the sixth pixel from the left edge, second row.
  lcd.print("Demo"); //print text on screen
  lcd.setCursor(6, 2); //set the cursor to the sixth pixel from the left edge, third row.
  lcd.print("Back"); //print text on screen  
}

void MenuSetup() { //submenu of Main menu - setup screen for Industruino
 channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 2;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("BackLight     ");
  lcd.setCursor(65, 0);
  lcd.print(backlightIntensity,1);
  lcd.setCursor(6, 1);
  lcd.print("Reset param.");
  lcd.setCursor(6, 2);
  lcd.print("Back");
}

void MenuParametersReset() {
  channel = 6;
  channelUpLimit = 5;
  channelLowLimit = 4;
  MenuID = 10;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Set system");
  lcd.setCursor(6, 1);
  lcd.print("to default");
  lcd.setCursor(6, 2);
  lcd.print("settings?");
  lcd.setCursor(6, 4);
  lcd.print("OK?");
  lcd.setCursor(6, 5);
  lcd.print("Cancel");
  
}

void MenuDemo() {
  channel = 0;
  channelUpLimit = 3;
  channelLowLimit = 0;
  MenuID = 3;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("DigitalOut");
  lcd.setCursor(6, 1);
  lcd.print("DigitalIn");
  lcd.setCursor(6, 2);
  lcd.print("AnalogIn");
  lcd.setCursor(6, 3);
  lcd.print("Back");
}



void MenuDigitalOut1() {
  SetOutput();
  channel = 0;
  channelUpLimit = 6;
  channelLowLimit = 0;
  MenuID = 1;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("D0");
  lcd.setCursor(6, 1);
  lcd.print("D1");
  lcd.setCursor(6, 2);
  lcd.print("D2");
  lcd.setCursor(6, 3);
  lcd.print("D3");
  lcd.setCursor(6, 4);
  lcd.print("D4");
  lcd.setCursor(6, 5);
  lcd.print("D5");
}

void MenuDigitalOut2() {
  channel = 0;
  channelUpLimit = 6;
  channelLowLimit = -1;
  MenuID = 2;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("D6");
  lcd.setCursor(6, 1);
  lcd.print("D7");
  lcd.setCursor(6, 2);
  lcd.print("D8");
  lcd.setCursor(6, 3);
  lcd.print("D9");
  lcd.setCursor(6, 4);
  lcd.print("D10");
  lcd.setCursor(6, 5);
  lcd.print("D11");

}

void MenuDigitalOut3() {
  channel = 0;
  channelUpLimit = 5;
  channelLowLimit = -1;
  MenuID = 3;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("D12");
  lcd.setCursor(6, 1);
  lcd.print("D14");
  lcd.setCursor(6, 2);
  lcd.print("D15");
  lcd.setCursor(6, 3);
  lcd.print("D16");
  lcd.setCursor(6, 4);
  lcd.print("D17");
  lcd.setCursor(6, 5);
  lcd.print("Back");
}


void MenuDigitalIn1() {
  SetInput();
  channel = 0;
  channelUpLimit = 1;
  channelLowLimit = 0;
  MenuID = 4;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  MenuDigitalIn1Live();
}

void MenuDigitalIn1Live() {
  lcd.setCursor(6, 0);
  lcd.print("D0  ");
  lcd.print(digitalRead(0));
  lcd.setCursor(6, 1);
  lcd.print("D1  ");
  lcd.print(digitalRead(1));
  lcd.setCursor(6, 2);
  lcd.print("D2  ");
  lcd.print(digitalRead(2));
  lcd.setCursor(6, 3);
  lcd.print("D3  ");
  lcd.print(digitalRead(3));
  lcd.setCursor(6, 4);
  lcd.print("D4  ");
  lcd.print(digitalRead(4));
  lcd.setCursor(6, 5);
  lcd.print("D5  ");
  lcd.print(digitalRead(5));


}

void MenuDigitalIn2() {
  channel = 0;
  channelUpLimit = 1;
  channelLowLimit = -1;
  MenuID = 5;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  MenuDigitalIn2Live();
}

void MenuDigitalIn2Live() {
  lcd.setCursor(6, 0);
  lcd.print("D6  ");
  lcd.print(digitalRead(6));
  lcd.setCursor(6, 1);
  lcd.print("D7  ");
  lcd.print(digitalRead(7));
  lcd.setCursor(6, 2);
  lcd.print("D8  ");
  lcd.print(digitalRead(8));
  lcd.setCursor(6, 3);
  lcd.print("D9  ");
  lcd.print(digitalRead(9));
  lcd.setCursor(6, 4);
  lcd.print("D10 ");
  lcd.print(digitalRead(10));
  lcd.setCursor(6, 5);
  lcd.print("D11 ");
  lcd.print(digitalRead(11));
}


void MenuDigitalIn3() {
  lcd.clear();
  channel = 5;
  channelUpLimit = 5;
  channelLowLimit = 4;
  MenuID = 6;
  MenuLevel = 3;
  enterPressed = 0;
  ScrollCursor();
  MenuDigitalIn2Live();
}

void MenuDigitalIn3Live() {
  lcd.setCursor(6, 0);
  lcd.print("D12 ");
  lcd.print(digitalRead(12));
  lcd.setCursor(6, 1);
  lcd.print("D14 ");
  lcd.print(digitalRead(14));
  lcd.setCursor(6, 2);
  lcd.print("D15 ");
  lcd.print(digitalRead(15));
  lcd.setCursor(6, 3);
  lcd.print("D16 ");
  lcd.print(digitalRead(16));
  lcd.setCursor(6, 4);
  lcd.print("D17 ");
  lcd.print(digitalRead(17));
  lcd.setCursor(6, 5);
  lcd.print("Back   ");
}


void AnalogOut1() {
  channel = 0;
  channelUpLimit = 6;
  channelLowLimit = 0;
  MenuID = 8;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("D0");
  lcd.setCursor(6, 1);
  lcd.print("D1");
  lcd.setCursor(6, 2);
  lcd.print("D3");
  lcd.setCursor(6, 3);
  lcd.print("D5");
  lcd.setCursor(6, 4);
  lcd.print("D6");
  lcd.setCursor(6, 5);
  lcd.print("D7");
}

void MenuAnalogIn1() {
  SetInput();
  channel = 0;
  channelUpLimit = 1;
  channelLowLimit = 0;
  MenuID = 7;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  MenuAnalogIn1Live();
}

void MenuAnalogIn1Live() {
  lcd.setCursor(6, 0);
  lcd.print("A0           ");
  lcd.setCursor(30, 0);
  lcd.print(analogRead(0));
  lcd.setCursor(6, 1);
  lcd.print("A6           ");
  lcd.setCursor(30, 1);
  lcd.print(analogRead(6));
  lcd.setCursor(6, 2);
  lcd.print("A7           ");
  lcd.setCursor(30, 2);
  lcd.print(analogRead(7));
  lcd.setCursor(6, 3);
  lcd.print("A8           ");
  lcd.setCursor(30, 3);
  lcd.print(analogRead(8));
  lcd.setCursor(6, 4);
  lcd.print("A9           ");
  lcd.setCursor(30, 4);
  lcd.print(analogRead(9));
  lcd.setCursor(6, 5);
  lcd.print("A10          ");
  lcd.setCursor(30, 5);
  lcd.print(analogRead(10));
}


void MenuAnalogIn2() {
  SetInput();
  channel = 1;
  channelUpLimit = 1;
  channelLowLimit = 0;
  MenuID = 8;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  MenuAnalogIn2Live();
}

void MenuAnalogIn2Live() {
  lcd.setCursor(6, 0);
  lcd.print("A11          ");
  lcd.setCursor(30, 0);
  lcd.print(analogRead(11));
  lcd.setCursor(6, 1);
  lcd.print("Back   ");
}



//---------------------------------------------------------------------------------------------------------------------------------------------------
//UI control logic, please edit this function to reflect the specific menus that your created above and your desired actions for each cursor position
//---------------------------------------------------------------------------------------------------------------------------------------------------



void Navigate()
{ 

if (valueEditing != 1){
  
  if (MenuLevel == 0) //check if current activated menu is the 'splash screen' (first level)
  {
      {
      if (enterPressed == 1) MenuMain(); //if enter is pressed load the 'Main menu'
      }
  }
  
   if (MenuLevel == 1){ //check if current activated menu is the 'Main menu' (first level)
    
    if (channel == 0 && enterPressed == 1) MenuSetup(); //if cursor is on the first row and enter is pressed load the 'Setup' menu
    if (channel == 1 && enterPressed == 1) MenuDemo(); //if cursor is on the second row and enter is pressed load the 'Demo' menu
    if (channel == 2 && enterPressed == 1) MenuSplash(); //if cursor is on the third row and enter is pressed load the 'splash screen'
     }
  
    
    
    if (MenuLevel == 2){
      
      if (MenuID == 2){
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
       {
        TargetValue = backlightIntensity; //copy variable to be edited to 'Target value'
        backlightIntensity = EditValue(); 
        analogWrite(backlightPin, (map(backlightIntensity, 0, 5, 255, 0)));
        EEPROM.write(0, backlightIntensity);
        
       }
        if (channel == 1 && enterPressed == 1) MenuParametersReset();
        if (channel == 2 && enterPressed == 1) MenuDemo();   
        }   
      if (MenuID == 3){
        if (channel == 0 && enterPressed == 1) MenuDigitalOut1(); 
        if (channel == 1 && enterPressed == 1) MenuDigitalIn1(); 
        if (channel == 2 && enterPressed == 1) MenuAnalogIn1(); 
        if (channel == 3 && enterPressed == 1) MenuMain(); 
        }
       }
     
    if (MenuLevel == 3){
      if (MenuID == 1){
        if (channel == 0 && buttonEnterState == HIGH) digitalWrite(D0, HIGH); 
        if (channel == 0 && buttonEnterState == LOW) digitalWrite(D0, LOW); 
        if (channel == 1 && buttonEnterState == HIGH) digitalWrite(D1, HIGH); 
        if (channel == 1 && buttonEnterState == LOW) digitalWrite(D1, LOW); 
        if (channel == 2 && buttonEnterState == HIGH) digitalWrite(D2, HIGH); 
        if (channel == 2 && buttonEnterState == LOW) digitalWrite(D2, LOW); 
        if (channel == 3 && buttonEnterState == HIGH) digitalWrite(D3, HIGH); 
        if (channel == 3 && buttonEnterState == LOW) digitalWrite(D3, LOW); 
        if (channel == 4 && buttonEnterState == HIGH) digitalWrite(D4, HIGH); 
        if (channel == 4 && buttonEnterState == LOW) digitalWrite(D4, LOW);
        if (channel == 5 && buttonEnterState == HIGH) digitalWrite(D5, HIGH); 
        if (channel == 5 && buttonEnterState == LOW) digitalWrite(D5, LOW);
        if (channel == 6) MenuDigitalOut2();
        } 
      if (MenuID == 2){
        if (channel == 0 && buttonEnterState == HIGH) digitalWrite(D6, HIGH); 
        if (channel == 0 && buttonEnterState == LOW) digitalWrite(D6, LOW); 
        if (channel == 1 && buttonEnterState == HIGH) digitalWrite(D7, HIGH); 
        if (channel == 1 && buttonEnterState == LOW) digitalWrite(D7, LOW); 
        if (channel == 2 && buttonEnterState == HIGH) digitalWrite(D8, HIGH); 
        if (channel == 2 && buttonEnterState == LOW) digitalWrite(D8, LOW); 
        if (channel == 3 && buttonEnterState == HIGH) digitalWrite(D9, HIGH); 
        if (channel == 3 && buttonEnterState == LOW) digitalWrite(D9, LOW); 
        if (channel == 4 && buttonEnterState == HIGH) digitalWrite(D10, HIGH); 
        if (channel == 4 && buttonEnterState == LOW) digitalWrite(D10, LOW); 
        if (channel == 5 && buttonEnterState == HIGH) digitalWrite(D11, HIGH); 
        if (channel == 5 && buttonEnterState == LOW) digitalWrite(D11, LOW); 
        if (channel == 6) MenuDigitalOut3();
        if (channel == -1) MenuDigitalOut1();
        }
      if (MenuID == 3){
        if (channel == 0 && buttonEnterState == HIGH) digitalWrite(D12, HIGH); 
        if (channel == 0 && buttonEnterState == LOW) digitalWrite(D12, LOW);
        if (channel == 1 && buttonEnterState == HIGH) digitalWrite(D14, HIGH); 
        if (channel == 1 && buttonEnterState == LOW) digitalWrite(D14, LOW); 
        if (channel == 2 && buttonEnterState == HIGH) digitalWrite(D15, HIGH); 
        if (channel == 2 && buttonEnterState == LOW) digitalWrite(D15, LOW); 
        if (channel == 3 && buttonEnterState == HIGH) digitalWrite(D16, HIGH); 
        if (channel == 3 && buttonEnterState == LOW) digitalWrite(D16, LOW); 
        if (channel == 4 && buttonEnterState == HIGH) digitalWrite(D17, HIGH); 
        if (channel == 4 && buttonEnterState == LOW) digitalWrite(D17, LOW);
        if (channel == 5 && enterPressed == 1) MenuDemo(); 
        if (channel == -1) MenuDigitalOut2();
        }
      if (MenuID == 4){
        if ((millis()-lastLCDredraw) > 300){
        MenuDigitalIn1Live();
        lastLCDredraw = millis();
        }
        if (channel == 1) MenuDigitalIn2();
        }
      if (MenuID == 5){
        if ((millis()-lastLCDredraw) > 300){
        MenuDigitalIn2Live();
        lastLCDredraw = millis();
        }
        if (channel == 1) MenuDigitalIn3();
        if (channel == -1) MenuDigitalIn1();
        }
      if (MenuID == 6){
        if ((millis()-lastLCDredraw) > 300){
        MenuDigitalIn3Live();
        lastLCDredraw = millis();
        }
        if (channel == 4) MenuDigitalIn2();
        if (channel == 5 && enterPressed == 1) MenuDemo(); 
        }    
      if (MenuID == 7){
        if ((millis()-lastLCDredraw) > 300){
        MenuAnalogIn1Live();
        lastLCDredraw = millis();
        }
        if (channel == 1) MenuAnalogIn2();
        }   
      if (MenuID == 8){
        if ((millis()-lastLCDredraw) > 300){
        MenuAnalogIn2Live();
        lastLCDredraw = millis();
        }
        if (channel == 0) MenuAnalogIn1();
        if (channel == 1 && enterPressed == 1) MenuDemo(); 
        }   
        
        
      if (MenuID == 10){
        if (channel == 4 && enterPressed == 1) ResetParameters();
        if (channel == 5 && enterPressed == 1) MenuSetup(); 
        } 
    
       }
       
 //dont remove this part
 if (channel != lastChannel && valueEditing != 1 && MenuID != 0){ //updates the cursor position if button counter changed and 'value editing mode' is not running
  ScrollCursor();
  }
}
}


//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
  
  
  float EditValue() //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditingInc = 1; //increment for each button press
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1){ //stays in 'value editing mode' until enter is pressed
  ReadButtons(); //check the buttons for any change
  if (channel != lastChannel){ //when up or down button is pressed
       if (channel < lastChannel && TargetValue <=4){ //if 'Up' button is pressed, and is within constraint range.
         TargetValue+= valueEditingInc; //increment target variable with pre-defined increment value
       }
       if (channel > lastChannel && TargetValue > 0){ //if 'Down' button is pressed, and is within constraint range.
         TargetValue-= valueEditingInc ; //decrement target variable with pre-defined increment value
       }
      //clear a section of a row to make space for updated value
      for (int i=60; i <= 70; i++){ 
      lcd.setCursor(i, row);
      lcd.print("   ");
       }
       //print updated value
       lcd.setCursor(66, row);
       Serial.println(TargetValue);
       lcd.print(TargetValue, 0);
       lastChannel = channel;
       }  
  delay(50);
  }  
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 1; //enable constrainment of button counter's range so to stay within the menu's range
  channelUpLimit = 2; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValue; //return the edited value to the main 'Navigate' UI control function for further processing
}
  
  
//--------------------------------------------------------------------------------------------------------------------------------------------- 
// Peripheral functions
//--------------------------------------------------------------------------------------------------------------------------------------------- 

void ReadButtons(){ //reads the 3 buttons from a single analoge pin, filters and decides which button was pressed
  
  ButtonsAnalogValue = analogRead(analogInPin);  
 
   if (ButtonsAnalogValue>400 && ButtonsAnalogValue< 700 )
  {
  buttonEnterState = HIGH;
  }
  else
  {
  buttonEnterState = LOW;
  }
  
    if (ButtonsAnalogValue>100 && ButtonsAnalogValue<400)
  {
   buttonUpState = HIGH;
  }
  else
  {
  buttonUpState = LOW;
  }
   
   
    if (ButtonsAnalogValue>700 && ButtonsAnalogValue<1000 )
  {
  buttonDownState = HIGH;
  }
  else
  {
  buttonDownState = LOW;
  }
  
  
  
  
      if (buttonEnterState == HIGH && prevBtnEnt == LOW)
      {
         if ((millis() - lastBtnEnt) > transEntInt) 
         {
         enterPressed = 1;
         }
         lastBtnEnt = millis();
         lastAdminActionTime = millis();
         //Serial.println("Enter Pressed");
       }
      prevBtnEnt = buttonEnterState;
      
      
           if (buttonUpState == HIGH && prevBtnUp == LOW)
      {
         if ((millis() - lastBtnUp) > transInt) 
         {
         channel--;
         }
         lastBtnUp = millis();
         lastAdminActionTime = millis();
         //Serial.println("Up Pressed");
       }
      prevBtnUp = buttonUpState;
      
      
           if (buttonDownState == HIGH && prevBtnDown == LOW)
      {
         if ((millis() - lastBtnDown) > transInt) 
         {
         channel++;
         }
         lastBtnDown = millis();
         lastAdminActionTime = millis();
         //Serial.println("Down Pressed");
       }
      prevBtnDown = buttonDownState;
      
       if (constrainEnc == 1){
   channel = constrain(channel, channelLowLimit, channelUpLimit);
   }
      
 }
 
 
 void SetOutput(){  // a simple function called to set a group of pins as outputs
  pinMode(D0, OUTPUT);
  pinMode(D1, OUTPUT); 
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT); 
  pinMode(D6, OUTPUT); 
  pinMode(D7, OUTPUT);
  pinMode(D8, OUTPUT);   
  pinMode(D9, OUTPUT); 
  pinMode(D10, OUTPUT);
  pinMode(D11, OUTPUT);
  pinMode(D12, OUTPUT);
  pinMode(D14, OUTPUT); 
  pinMode(D15, OUTPUT);  
  pinMode(D16, OUTPUT);
  pinMode(D17, OUTPUT); 
 }
 
void SetInput(){  // a simple function called to set a group of pins as inputs
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);  
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT); 
  pinMode(D6, INPUT); 
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);  
  pinMode(D9, INPUT); 
  pinMode(D10, INPUT);
  pinMode(D11, INPUT);
  pinMode(D12, INPUT);
  pinMode(D14, INPUT); 
  pinMode(D15, INPUT);  
  pinMode(D16, INPUT);
  pinMode(D17, INPUT); 
 }
 
 void ResetParameters(){ //resets the setup parameters of Industruino and saves the settings to EEPROM
   
   backlightIntensity = backlightIntensityDef; //load the default backlight intensity value
   analogWrite(backlightPin, (map(backlightIntensity, 0, 5, 255, 0))); //map the value (from 0-5) to a corresponding PWM value (0-255) and update the output 
   EEPROM.write(0, backlightIntensity); //save the current backlight intensity to EEPROM
   MenuSetup(); //return to the setup menu
 }
 
 

//---------------------------------------------------------------------------------------------------------------------------------------------
// UI core functions
//--------------------------------------------------------------------------------------------------------------------------------------------- 
 


void ScrollCursor() //makes the cursor move
{
  lastChannel = channel; //keep track button counter changes
  for (int i=0; i <= 6; i++){ //clear the whole column when redrawing a new cursor
      lcd.setCursor(coll, i);
      lcd.print(" ");
   } 
  lcd.setCursor(coll, channel); //set new cursor position
  lcd.print(">"); //draw cursor
 
  }

