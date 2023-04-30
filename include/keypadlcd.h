#ifndef LCD_H
#define LCD_H

#include <LiquidCrystal.h>
#include "config.h"
#include <Arduino.h>

/*******************************************************

This program is based on test the LCD panel and the buttons on D1 Robot LCD Keypad
Michael Jonathan, February 2019

********************************************************/

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

/**
 * @brief KeypadLCDControl class
*/
class KeypadLCDControl{
    public:
        #ifdef ESP8266
            KeypadLCDControl() : lcd(D8, D9, D4, D5, D6, D7), buttonPin(PIN_A0) {}
        #endif
        #ifdef ARDUINO_AVR_UNO
            KeypadLCDControl() : lcd(8, 9, 4, 5, 6, 7), buttonPin(PIN_A0) {}
        #endif
        int read_LCD_buttons();
        void setup_hook();
        void loop_hook();
    private:
        LiquidCrystal lcd;
        int buttonPin;
        int selectedKey;
        int adc_key_in;
        int lcd_key;
        int previousKey;
};

// select the pins used on the LCD panel
/*
#ifdef ESP8266
LiquidCrystal lcd(D8, D9, D4, D5, D6, D7);//Wemos D1 R1
#endif

#ifdef ARDUINO_AVR_UNO
LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // Arduino Uno
#endif
*/



// read the buttons
int KeypadLCDControl::read_LCD_buttons()
{
  adc_key_in = analogRead(PIN_A0);      // read the value from the sensor 
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  selectedKey = btnNONE;
  if (adc_key_in > 1000) selectedKey = btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)  selectedKey = btnRIGHT;  
  if (adc_key_in < 250) selectedKey = btnUP; 
  if (adc_key_in < 450)  selectedKey = btnDOWN; 
  if (adc_key_in < 650)  selectedKey = btnLEFT; 
  if (adc_key_in < 850)  selectedKey = btnSELECT;  

  return selectedKey;
}

void KeypadLCDControl::setup_hook()
{
  Serial.begin(SERIAL_BAUD_RATE);
  Serial.println(WATER_BOOT_TEXT);
  lcd.begin(LCD_ROWS, LCD_COLS);              // start the library
  //lcd.setCursor(0, 0);
  //lcd.print("Created By");
  //lcd.setCursor(0,1);
  //lcd.print("Michael Jonathan");
  //delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Push the buttons"); // print a simple message
}
 
void KeypadLCDControl::loop_hook()
{
 lcd.setCursor(9,1);            // move cursor to second line "1" and 9 spaces over
 lcd.print(millis()/1000);      // display seconds elapsed since power-up


 lcd.setCursor(0,1);            // move to the begining of the second line
 lcd_key = read_LCD_buttons();  // read the buttons

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {
   case btnRIGHT:
     {
     lcd.print("RIGHT ");
     break;
     }
   case btnLEFT:
     {
     lcd.print("LEFT   ");
     break;
     }
   case btnUP:
     {
     lcd.print("UP    ");
     break;
     }
   case btnDOWN:
     {
     lcd.print("DOWN  ");
     break;
     }
   case btnSELECT:
     {
     lcd.print("SELECT");
     break;
     }
     case btnNONE:
     {
     lcd.print("NONE  ");
     break;
     }
 }

}

//to clear the LCD display, use the comment below
//lcd.clear(); 

#endif