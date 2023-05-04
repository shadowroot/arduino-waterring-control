#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"
#include "keypadlcd.h"
#include "waterring.h"
#include "menu.h"
#include "logger.h"
#include "async_comm.h"

DynamicJsonDocument doc(1024);
AsyncComm asyncComm(&Serial, &doc, "waterring");
Logger logger(&asyncComm);
WaterringComm waterringComm(&Serial, &doc, &logger , "waterring");
SoilMoistureSensor soilMoistureSensor1(SOIL_MOISTURE_SENSOR1_PIN, &logger, &waterringComm, "soil_moisture_sensor1");
SoilMoistureSensor soilMoistureSensor2(SOIL_MOISTURE_SENSOR2_PIN, &logger, &waterringComm, "soil_moisture_sensor2");
Pump pump(PUMP_PIN, &logger, &waterringComm, "main_water_pump");
KeypadLCDControl keypadLCDControl;
Waterring waterring(&logger, &waterringComm, &pump, &soilMoistureSensor1, &soilMoistureSensor2);
Menu menu(&keypadLCDControl, &waterring);

void setup()
{
  asyncComm.setLogger(&logger);
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
