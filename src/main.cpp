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
Wattering waterring(soilMoistureSensor1, soilMoistureSensor2, pump);
Menu menu(keypadLCDControl, waterring);

void setup()
{
  keypadLCDControl.setup_hook();
  waterring.setup_hook();
  menu.setup_hook();
}
 
void loop()
{
  keypadLCDControl.loop_hook();
  waterring.loop_hook();
  menu.loop_hook();
}
