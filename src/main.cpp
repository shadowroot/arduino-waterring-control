//Sample using LiquidCrystal library
#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"
#include "keypadlcd.h"
#include "wattering.h"
#include "menu.h"

SoilMoistureSensor soilMoistureSensor1(SOIL_MOISTURE_SENSOR1_PIN);
SoilMoistureSensor soilMoistureSensor2(SOIL_MOISTURE_SENSOR2_PIN);
Pump pump(PUMP_PIN);
KeypadLCDControl keypadLCDControl;
Menu menu(keypadLCDControl);
Wattering waterring(soilMoistureSensor1, soilMoistureSensor2, pump);

void setup()
{
  waterring.setup_hook();
  menu.setup_hook();
}
 
void loop()
{
  waterring.loop_hook();
  menu.loop_hook();
}

//to clear the LCD display, use the comment below
//lcd.clear(); 