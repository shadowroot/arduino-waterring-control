#include <Arduino.h>
#include <LiquidCrystal.h>
#include "config.h"
#include "keypadlcd.h"
#include "waterring.h"
#include "menu.h"
#include "async_comm.h"

DynamicJsonDocument doc(1024);
WaterringComm waterringComm((Stream *) &Serial, &doc, "waterring");
SoilMoistureSensor soilMoistureSensor1(SOIL_MOISTURE_SENSOR1_PIN, &waterringComm, "soil_moisture_sensor1");
SoilMoistureSensor soilMoistureSensor2(SOIL_MOISTURE_SENSOR2_PIN, &waterringComm, "soil_moisture_sensor2");
Pump pump(PUMP_PIN, &waterringComm, "main_water_pump");
KeypadLCDControl keypadLCDControl;
WaterringPump2MoistureSensor waterring(&waterringComm, &pump, &soilMoistureSensor1, &soilMoistureSensor2);
Menu menu(&keypadLCDControl, &waterring);

void setup()
{
  waterringComm.setup_hook();
  keypadLCDControl.setup_hook();
  waterring.setup_hook();
  menu.setup_hook();
}
 
void loop()
{
  waterringComm.loop_hook();
  keypadLCDControl.loop_hook();
  waterring.loop_hook();
  menu.loop_hook();
}
