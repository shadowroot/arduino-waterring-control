#ifndef LCD_H
#define LCD_H

#include <LiquidCrystal.h>

/*******************************************************

This program is based on test the LCD panel and the buttons on D1 Robot LCD Keypad
Michael Jonathan, February 2019

********************************************************/

enum ButtonPressed{ RIGHT, UP, DOWN, LEFT, SELECT, NONE };

/**
 * @brief KeypadLCDControl class
*/
class KeypadLCDControl{
    public:
        #ifdef ESP8266
            KeypadLCDControl() : lcd(D8, D9, D4, D5, D6, D7), buttonPin(PIN_A0) {}
        #endif
        KeypadLCDControl(int rs_pin, int en_pin, int d4_pin, int d5_pin, int d6_pin, int d7_pin, int buttonPin) : lcd(rs_pin, en_pin, d4_pin, d5_pin, d6_pin, d7_pin), buttonPin(buttonPin) {}
        ButtonPressed read_LCD_buttons();
        void setup_hook();
        void loop_hook();
        LiquidCrystal & getLCD(){
          return lcd;
        }
        void clear(){
          lcd.clear();
        }
        void printTextUp(const char * text){
          lcd.setCursor(1,0);
          lcd.print(text);
        }
        void printTextDown(const char * text){
          lcd.setCursor(0,1);
          lcd.print(text);
        }
        int findTextMiddle(const char * text){
          int len = strlen(text);
          if(len > LCD_COLS-2){
            return 1;
          }
          return (LCD_COLS - len-2)/2;
        }
        void printNavPrev(){
          lcd.setCursor(0,0);
          lcd.print(LEFT_SYMBOL);
        }
        void printNavNext(){
          lcd.setCursor(LCD_COLS-1,0);
          lcd.print(RIGHT_SYMBOL);
        }
        ButtonPressed keyPressed(){
          if(selectedKey != previousKey){
            return selectedKey;
          }
          return ButtonPressed::NONE;
        }
    private:
        LiquidCrystal lcd;
        int buttonPin;
        ButtonPressed selectedKey;
        int adc_key_in;
        int lcd_key;
        ButtonPressed previousKey;
        unsigned long lastReadTime;
};

// read the buttons
ButtonPressed KeypadLCDControl::read_LCD_buttons()
{
  adc_key_in = analogRead(buttonPin);      // read the value from the sensor 
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  selectedKey = ButtonPressed::NONE;
  if (adc_key_in > 1000) selectedKey = ButtonPressed::NONE; // We make this the 1st option for speed reasons since it will be the most likely result
  // For V1.1 us this threshold
  if (adc_key_in < 50)  selectedKey = ButtonPressed::RIGHT;  
  if (adc_key_in < 250) selectedKey = ButtonPressed::UP; 
  if (adc_key_in < 450)  selectedKey = ButtonPressed::DOWN; 
  if (adc_key_in < 650)  selectedKey = ButtonPressed::LEFT; 
  if (adc_key_in < 850)  selectedKey = ButtonPressed::SELECT;  

  return selectedKey;
}

void KeypadLCDControl::setup_hook()
{
  lcd.begin(LCD_ROWS, LCD_COLS);              // start the library
}
 
void KeypadLCDControl::loop_hook()
{
  if(millis() - lastReadTime >= 100){
    previousKey = selectedKey;
    read_LCD_buttons();  // read the buttons
    lastReadTime = millis();
  }
}

//to clear the LCD display, use the comment below
//lcd.clear(); 

#endif