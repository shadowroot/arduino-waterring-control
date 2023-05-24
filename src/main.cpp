#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoHA.h>
#include "config.h"
#include "waterring.h"
#include "keypadlcd.h"

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);


HASwitch main_water_pump("main_water_pump");
HASwitch auto_water_pump("auto_water_switch");
SoilMoistureSensor sensor1(SOIL_MOISTURE_SENSOR1_PIN, "sensor1", 1800, 40);
SoilMoistureSensor sensor2(SOIL_MOISTURE_SENSOR2_PIN, "sensor2", 1800, 40);
Pump pump(PUMP_PIN, "pump");
MoistureSensorPower moistureSensorPower(GPIO_NUM_3, "moistureSensorPower");
WaterringPump2MoistureSensor watering(&pump, &sensor1, &sensor2);


void onPumpSwitchCommand(bool state, HASwitch* sender)
{
    if(state)
    {
        // turn on the pump
        watering.manualOn();
    }
    else
    {
        // turn off the pump
        watering.manualOff();
    }
    sender->setState(state); // report state back to the Home Assistant
}

void onAutoSwitchCommand(bool state, HASwitch* sender)
{
    if(state)
    {
        // turn off manual watering
        watering.automatedOn();
    }

    sender->setState(state); // report state back to the Home Assistant
}

void initWiFi() {
  byte mac[6];
  WiFi.macAddress(mac);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  device.setUniqueId(mac, sizeof(mac));

  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
    watering.setup_hook();
    initWiFi();
    // set device's details (optional)
    device.setName("PumpController");
    device.setSoftwareVersion("0.0.1");
    // handle switch state - turn on/off the pump - not command for each action, but just state changer
    main_water_pump.onCommand(onPumpSwitchCommand);
    main_water_pump.setName("Pump on/off"); // optional
    auto_water_pump.onCommand(onAutoSwitchCommand);
    auto_water_pump.setName("Auto/manual"); // optional
    mqtt.begin(BROKER_ADDR, BROKER_USERNAME, BROKER_PASSWORD);
}

void loop() {
    mqtt.loop();
    watering.loop_hook();
}