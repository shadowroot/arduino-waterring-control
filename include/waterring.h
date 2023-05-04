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


//start of sensor and actuators

class SoilMoistureSensor{
    public:
        SoilMoistureSensor(int pin, Logger * log, AsyncComm * asyncComm, const char * name = "Soil_moisture_sensor", int readIntervalSeconds=1800, int treashold=40) : pin(pin), log(log), asyncComm(asyncComm), name(name), readIntervalSeconds(readIntervalSeconds), lastRead(0), treashold(treashold) {}
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
        Logger * log;
        AsyncComm * asyncComm;
        const char * name;
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
        //creating and sending event message
        asyncComm->createEventMessage();
        asyncComm->addMessageField("device", name);
        asyncComm->addMessageField("moisture_percentage", percentage);
        asyncComm->addMessageField("moisture_raw", analogValue);
        asyncComm->sendMsg();
        //end of creating and sending event message
        log->debug("Soil moisture sensor %s read: %d", name, percentage);

        lastRead = millis();
    }
}

class Relay{
    public:
        Relay(int pin, Logger* log, AsyncComm * asyncComm, const char * name = "relay"): pin(pin), log(log), asyncComm(asyncComm), name(name){
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
        unsigned long getRunningTime(){
            return millis() - runningTime;
        }
    protected:
        int pin;
        unsigned long lastRunning;
        unsigned long runningTime;
        bool running;
        const char * name;
        Logger* log;
        AsyncComm * asyncComm;
};

void Relay::setup_hook(){
    log->info("Relay %s setup", name);
}

void Relay::loop_hook(){
    log->debug("Relay %s hook", name);
}

void Relay::on(){
    log->debug("Relay %s set on", name);
    digitalWrite(pin, HIGH);
    lastRunning = millis();
    running = true;
    //creating and sending event message
    asyncComm->createEventMessage();
    asyncComm->addMessageField("device", name);
    asyncComm->addMessageField("state", "on");
    asyncComm->sendMsg();
    //end of creating and sending event message
}

void Relay::off(){
    log->debug("Relay %s set off", name);
    digitalWrite(pin, LOW);
    runningTime = millis() - lastRunning;
    running = false;
    //creating and sending event message
    asyncComm->createEventMessage();
    asyncComm->addMessageField("device", name);
    asyncComm->addMessageField("state", "off");
    asyncComm->sendMsg();
    //end of creating and sending event message
}


class Pump : public Relay{
    public:
        Pump(int pin, Logger * log, AsyncComm * asyncComm, const char * name = "pump") : Relay(pin, log, asyncComm, name) {}
};


class MoistureSensorPower : public Relay{
    public:
        MoistureSensorPower(int pin, Logger * log, AsyncComm * asyncComm, const char * name = "moisturepower") : Relay(pin, log, asyncComm, name) {}
};

class AnalogReader{
    public:
        AnalogReader(int pin, Logger * log, AsyncComm * asyncComm, const char * name = "AnalogReader") : pin(pin), log(log), asyncComm(asyncComm), name(name) {}
        void setup_hook();
        void loop_hook(){}
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, INPUT);
        }
        void readValueRPC();
    protected:
        int pin;
        Logger * log;
        AsyncComm * asyncComm;
        const char * name;
};

void AnalogReader::setup_hook(){
    log->info("AnalogReader %s setup", name);
    setPin(pin);
}

void AnalogReader::readValueRPC(){
    log->debug("AnalogReader %s hook", name);
    int value = analogRead(pin);
    asyncComm->createRPCMessage();
    asyncComm->addRPCResult(value);
    asyncComm->sendMsg();
}

//end of sensors

/**
 * @brief WaterringState class
*/
class Waterring{
    public:
        Waterring( Logger * log, WaterringComm * waterringComm, Pump* pump, SoilMoistureSensor* soilMoistureSensor1 = NULL, SoilMoistureSensor* soilMoistureSensor2 = NULL) : log(log), waterringComm(waterringComm), currentWaterringState(AUTOMATED), soilMoistureSensor1(soilMoistureSensor1), soilMoistureSensor2(soilMoistureSensor2), pump(pump){}
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
        Logger * log;
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




#endif //WATERING_H