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

enum SensorsState{
    FAIL_SENSOR1,
    FAIL_SENSOR2,
    FAIL_ALL_SENSORS,
    OK,
};

/**
 * @brief WatteringState class
*/
class Wattering{
    public:
        Wattering(const SoilMoistureSensor& soilMoistureSensor1, const SoilMoistureSensor& soilMoistureSensor2) : currentWatteringState(AUTOMATED), soilMoistureSensor1(soilMoistureSensor1), soilMoistureSensor2(soilMoistureSensor2){}
        void setup_hook();
        void loop_hook();
        WatteringState getWatteringState(){
            return currentWatteringState;
        }
        void setWatteringCycleSeconds(unsigned long wateringCycleSeconds){
            this->wateringCycleSeconds = wateringCycleSeconds;
        }
        void manualOn(){
            currentWatteringState = MANUAL_ON;
            pump.on();
        }
        void manualOff(){
            currentWatteringState = MANUAL_OFF;
            pump.off();
        }
    private:
        WatteringState currentWatteringState;
        unsigned long startWateringTime;
        unsigned long lastWateringTime;
        unsigned long wateringCycleSeconds;
        SoilMoistureSensor soilMoistureSensor1;
        SoilMoistureSensor soilMoistureSensor2;
        Pump pump;
        SensorsState sensorsState;
};

void Wattering::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    soilMoistureSensor1.setup_hook();
    soilMoistureSensor2.setup_hook();
}

void Wattering::loop_hook(){
    soilMoistureSensor1.loop_hook();
    soilMoistureSensor2.loop_hook();
    switch(currentWatteringState){
        case MANUAL_OFF:
            break;
        case MANUAL_ON:
            break;
        case AUTOMATED_WATERING:
            if(millis() - startWateringTime > wateringCycleSeconds * 1000){
                currentWatteringState = AUTOMATED;
                if(soilMoistureSensor1.isError() && soilMoistureSensor2.isError()){
                    sensorsState = FAIL_ALL_SENSORS;
                }
                else if(soilMoistureSensor1.isError()){
                    sensorsState = FAIL_SENSOR1;
                }
                else if(soilMoistureSensor2.isError()){
                    sensorsState = FAIL_SENSOR2;
                }
                else{
                    sensorsState = OK;
                }
                lastWateringTime = millis();
            }
            break;
        case AUTOMATED:
            if(soilMoistureSensor1.isDry() || soilMoistureSensor2.isDry()){
                currentWatteringState = AUTOMATED_WATERING;
                soilMoistureSensor1.savePercentage();
                soilMoistureSensor2.savePercentage();
                startWateringTime = millis();
            }
            else if(millis() - lastWateringTime > wateringCycleSeconds * 1000){
                currentWatteringState = AUTOMATED_WATERING;
                soilMoistureSensor1.savePercentage();
                soilMoistureSensor2.savePercentage();
                startWateringTime = millis();
            }
            break;
    }
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
        bool isError(){
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

class Pump{
    public:
        Pump(int pin) : pin(pin) {}
        void setup_hook();
        void loop_hook();
        void on();
        void off();
    private:
        int pin;
        unsigned long lastRunning;
        unsigned long runningTime;
        bool running;
};

void Pump::setup_hook(){
    pinMode(pin, OUTPUT);
}

void Pump::loop_hook(){
}

void Pump::on(){
    digitalWrite(pin, HIGH);
    lastRunning = millis();
    running = true;
}

void Pump::off(){
    digitalWrite(pin, LOW);
    runningTime = millis() - lastRunning;
    running = false;
}


#endif