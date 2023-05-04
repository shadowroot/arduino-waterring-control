#ifndef WATERRING_H
#define WATERRING_H

#include "keypadlcd.h"
#include "comm.h"
#include <Arduino.h>

enum WaterringState{
    MANUAL_OFF,
    MANUAL_ON,
    MANUAL_CYCLE,
    AUTOMATED_WATERING,
    AUTOMATED,
};
//might be just disconnected
enum SensorsState{
    FAIL_SENSOR1,
    FAIL_SENSOR2,
    FAIL_ALL_SENSORS,
    OK,
};

/**
 * @brief WaterringState class
*/
class Waterring{
    public:
        Waterring( Pump* pump, SoilMoistureSensor* soilMoistureSensor1 = NULL, SoilMoistureSensor* soilMoistureSensor2 = NULL) : currentWaterringState(AUTOMATED), soilMoistureSensor1(soilMoistureSensor1), soilMoistureSensor2(soilMoistureSensor2), pump(pump){}
        void setup_hook();
        void loop_hook();
        WaterringState getWaterringState(){
            return currentWaterringState;
        }
        void setWaterringCycleSeconds(unsigned long wateringCycleSeconds){
            this->wateringCycleSeconds = wateringCycleSeconds;
        }
        void manualOn(){
            currentWaterringState = MANUAL_ON;
            pump->on();
        }
        void manualOff(){
            currentWaterringState = MANUAL_OFF;
            pump->off();
        }
        void manualCycle(){
            currentWaterringState = MANUAL_CYCLE;
            pump->on();
            startWateringTime = millis();
        }
        int getWaterringCycleSeconds(){
            return wateringCycleSeconds;
        }
        void setWaterringCycleSeconds(int wateringCycleSeconds){
            this->wateringCycleSeconds = wateringCycleSeconds;
        }
    private:
        WaterringState currentWaterringState;
        unsigned long startWateringTime;
        unsigned long lastWateringTime;
        unsigned long wateringCycleSeconds;
        unsigned long tmpWateringCycleSeconds;
        SoilMoistureSensor *soilMoistureSensor1;
        SoilMoistureSensor *soilMoistureSensor2;
        Pump *pump;
        SensorsState sensorsState;
        WaterringComm *waterringComm;
        Log * log;
};

void Waterring::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    soilMoistureSensor1->setup_hook();
    soilMoistureSensor2->setup_hook();
}

void Waterring::loop_hook(){
    soilMoistureSensor1->loop_hook();
    soilMoistureSensor2->loop_hook();
    switch(currentWaterringState){
        case AUTOMATED_WATERING:
            if(millis() - startWateringTime > wateringCycleSeconds * 1000){
                currentWaterringState = AUTOMATED;
                pump->off();
                //either not working or disconnected
                if((soilMoistureSensor1 && soilMoistureSensor1->isError()) && (soilMoistureSensor2 && soilMoistureSensor2->isError())){
                    sensorsState = FAIL_ALL_SENSORS;
                }
                else if(soilMoistureSensor1 && soilMoistureSensor1->isError()){
                    sensorsState = FAIL_SENSOR1;
                }
                else if(soilMoistureSensor2 && soilMoistureSensor2->isError()){
                    sensorsState = FAIL_SENSOR2;
                }
                else{
                    sensorsState = OK;
                }
                lastWateringTime = millis();
            }
            break;
        case AUTOMATED:
            if((soilMoistureSensor1 && soilMoistureSensor1->isDry()) || (soilMoistureSensor2 && soilMoistureSensor2->isDry())){
                currentWaterringState = AUTOMATED_WATERING;
                soilMoistureSensor1->savePercentage();
                soilMoistureSensor2->savePercentage();
                pump->on();
                startWateringTime = millis();
            }
            else if(millis() - lastWateringTime > wateringCycleSeconds * 1000){
                currentWaterringState = AUTOMATED_WATERING;
                if(soilMoistureSensor1) soilMoistureSensor1->savePercentage();
                if(soilMoistureSensor2) soilMoistureSensor2->savePercentage();
                pump->on();
                startWateringTime = millis();
            }
            break;
        case MANUAL_CYCLE:
            if(millis() - startWateringTime > wateringCycleSeconds * 1000){
                pump->off();
                lastWateringTime = millis();
                currentWaterringState = AUTOMATED;
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

class Relay{
    public:
        Relay(int pin, Log* log, const char * name = "relay"): log(log), name(name){
            setPin(pin);
        }
        void setup_hook();
        void loop_hook();
        void on();
        void off();
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, OUTPUT);
        }
    protected:
        int pin;
        unsigned long lastRunning;
        unsigned long runningTime;
        bool running;
        const char * name;
        Log* log;
};

void Relay::setup_hook(){
    log->info("Relay %s setup", name);
}

void Relay::loop_hook(){
    log->debug("Relay %s hook", name);
}

void Relay::on(){
    digitalWrite(pin, HIGH);
    lastRunning = millis();
    running = true;
    log->debug("Relay %s set on", name);
}

void Relay::off(){
    digitalWrite(pin, LOW);
    runningTime = millis() - lastRunning;
    running = false;
    log->debug("Relay %s set off", name);
}


class Pump : public Relay{
    public:
        Pump(int pin, Log * log, const char * name = "pump") :  Relay(pin, log, name) {}
};


class MoistureSensorPower : public Relay{
    public:
        MoistureSensorPower(int pin, Log * log, const char * name = "moisturepower") :  Relay(pin, log, name) {}
};

class AnalogReader{
    public:
        AnalogReader(int pin, Log * log, AsyncComm * asyncComm, const char * name = "AnalogReader") : pin(pin), log(log), asyncComm(asyncComm), name(name) {}
        void setup_hook();
        void loop_hook(){}
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, INPUT);
        }
        void readValue();
    protected:
        int pin;
        Log * log;
        AsyncComm * asyncComm;
        const char * name;
};

void AnalogReader::setup_hook(){
    setPin(pin);
    log->info("AnalogReader %s setup", name);
}

void AnalogReader::readValue(){
    log->debug("AnalogReader %s hook", name);
    int value = analogRead(pin);
    asyncComm->createRPCMessage();
    asyncComm->addRPCResult(value);
    asyncComm->sendMsg();
}


#endif //WATERING_H