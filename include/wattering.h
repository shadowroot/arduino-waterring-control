#ifndef WATTERING_H
#define WATTERING_H
#include "keypadlcd.h"
#include <Arduino.h>

enum WatteringState{
    MANUAL_OFF,
    MANUAL_ON,
    AUTOMATED_WATERING,
    AUTOMATED,
};

/**
 * @brief WatteringState class
*/
class Wattering{
    public:
        Wattering(const SoilMoistureSensor& soilMoistureSensor1, const SoilMoistureSensor& soilMoistureSensor2) : currentWatteringState(AUTOMATED),{}
        void setup_hook();
        void loop_hook();
        WatteringState getWatteringState(){
            return currentWatteringState;
        }
        void setWatteringCycleSeconds(unsigned long wateringCycleSeconds){
            this->wateringCycleSeconds = wateringCycleSeconds;
        }
    private:
        WatteringState currentWatteringState;
        unsigned long startWateringTime;
        unsigned long lastWateringTime;
        unsigned long wateringCycleSeconds;
        SoilMoistureSensor soilMoistureSensor1;
        SoilMoistureSensor soilMoistureSensor2;
};

void Wattering::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(WATER_BOOT_TEXT);
    soilMoistureSensor1.setup_hook();
    soilMoistureSensor2.setup_hook();
}

void Wattering::loop_hook(){
    soilMoistureSensor1.loop_hook();
    soilMoistureSensor2.loop_hook();

}



class SoilMoistureSensor{
    public:
        SoilMoistureSensor(int pin, int readIntervalSeconds=1800, int treashold=40) : pin(pin), readIntervalSeconds(readIntervalSeconds), lastRead(0), treashold(treashold) {}
        void setup_hook();
        void loop_hook();
        void getSoilMoisture();
        int getPercentage(){
            return percentage;
        }
        bool isDry(){
            return percentage <= treashold;
        }
        void savePercentage(){
            lastPercentage = percentage;
        }
        bool indicateError(){
            return lastPercentage <= percentage;
        }
    private:
        int pin;
        int percentage;
        unsigned long lastRead;
        int treashold;
        int readIntervalSeconds;
        int lastPercentage;
};

void SoilMoistureSensor::setup_hook(){
    pinMode(pin, INPUT);
}

void SoilMoistureSensor::loop_hook(){
    getSoilMoisture();
}

void SoilMoistureSensor::getSoilMoisture(){
    if(millis() - lastRead < (readIntervalSeconds * 1000)){
        int analogValue = analogRead(pin);
        percentage = map(analogValue, 0, 1023, 0, 100);
        lastRead = millis();
    }
}

#endif