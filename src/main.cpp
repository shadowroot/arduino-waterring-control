#include <WiFi.h>
#include <ArduinoHA.h>
#include "config.h"
#include "waterring.h"
#include <Wire.h>
#include "menu.h"
#include "keypadlcd.h"
#include <Arduino.h>
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);


HASwitch main_water_pump("main_water_pump");
HASwitch auto_water_pump("auto_water_switch");
SoilMoistureSensor sensor1(SOIL_MOISTURE_SENSOR1_PIN, "sensor1", 1800, 40);
SoilMoistureSensor sensor2(SOIL_MOISTURE_SENSOR2_PIN, "sensor2", 1800, 40);
Pump pump(PUMP_PIN, "pump");
MoistureSensorPower moistureSensorPower(MOISTURE_SENSOR_POWER_PIN, "moistureSensorPower");
WaterringPump2MoistureSensor watering(&pump, &sensor1, &sensor2);
// KeypadLCDControl keypadLCDControl(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN, BUTTON_PIN);
// Menu menu(&keypadLCDControl, &watering);

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
  Serial.print("Connectin to SSID:");
  Serial.println(WIFI_SSID);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println("Setup start");
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
    watering.setup_hook();
    Serial.println("Setup done");

    // //LCD 1306
    // display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
    // // Show image buffer on the display hardware.
    // // Since the buffer is intialized with an Adafruit splashscreen
    // // internally, this will display the splashscreen.
    // display.display();
    // delay(1000);

    // // Clear the buffer.
    // display.clearDisplay();
    //     // text display tests
    // display.setTextSize(1);
    // display.setTextColor(SSD1306_WHITE);
    // display.setCursor(0,0);
    // display.print("Connecting to SSID\n ");
    // display.print(WIFI_SSID);
    // display.print(" ");
    // display.print("connected!");
    // display.println("IP: 10.0.1.23");
    // display.println("Sending val #0");
    // display.setCursor(0,0);
    // display.display(); // actually display all of the above
    // menu.setup_hook();
}

void loop() {
    Serial.println("loop start");
    mqtt.loop();
    /*
    if(digitalRead(BUTTON_PIN) == LOW){
        watering.manualSwitch();
        while(digitalRead(BUTTON_PIN) == LOW){}
    }
    */
    // menu.loop_hook();
    Serial.println("loop end");
}