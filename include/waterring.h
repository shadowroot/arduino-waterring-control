#ifndef WATERRING_H
#define WATERRING_H

#include "keypadlcd.h"
#include "comm.h"
#include <Arduino.h>
#include <map>
#include <vector>

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

class AsyncCommDevice{
    public:
        AsyncCommDevice(AsyncComm * asyncComm, const char * deviceName = "AsyncCommDevice") : asyncComm(asyncComm), deviceName(deviceName) {}
        //virtual void processEvent(){
        void processEvent(){
            if(asyncComm->isEvent()){
                asyncComm->logInfo("Processing event");
                if(strcmp(asyncComm->getDoc()["to"], deviceName) == 0){
                    determineAction();
                }
            }
        }
        //virtual void determineAction(){
        void determineAction(){
            const char * action = asyncComm->getDocKey("action"); 
            if( action != NULL){
                if(isActionPresent(action)){
                    callAction(action);
                    isProcessed = true;
                }
            }
            isProcessed = false;
        }
        bool isProcessedSuccessfully(){
            return isProcessed;
        }
        void addAction(const char * action, void (*f)()){
            actions[action] = f;
        }
        void callAction(const char * action){
            actions[action]();
        }
        bool isActionPresent(const char * action){
            return actions.find(action) != actions.end();
        }
    protected:
        AsyncComm * asyncComm;
        const char * deviceName;
        bool isProcessed;
        std::map<const char *, void (*)()> actions;
};

class SoilMoistureSensor : public AsyncCommDevice{
    public:
        SoilMoistureSensor(int pin, AsyncComm * asyncComm, const char * deviceName = "Soil_moisture_sensor", int readIntervalSeconds=1800, int treashold=40) : AsyncCommDevice(asyncComm, deviceName), pin(pin), readIntervalSeconds(readIntervalSeconds), lastRead(0), treashold(treashold) {}
        void setup_hook();
        void loop_hook();
        void getSoilMoisture();
        void getSoilMoistureInterval();
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
    protected:
        int pin;
        int percentage;
        unsigned long lastRead;
        int treashold;
        int readIntervalSeconds;
        int lastPercentage;
        
};

void SoilMoistureSensor::setup_hook(){
    pinMode(pin, INPUT);
    addAction("getSoilMoisture", (void (*)())&getSoilMoisture);
}

void SoilMoistureSensor::loop_hook(){
    getSoilMoistureInterval();
    processEvent();
}

void SoilMoistureSensor::getSoilMoisture(){
    int analogValue = analogRead(pin);
    percentage = map(analogValue, 0, 1023, 0, 100);
    //creating and sending event message
    asyncComm->createEventMessage();
    asyncComm->addMessageField("device", deviceName);
    asyncComm->addMessageField("moisture_percentage", percentage);
    asyncComm->addMessageField("moisture_raw", analogValue);
    asyncComm->sendMsg();
    //end of creating and sending event message
    asyncComm->logDebug("Soil moisture sensor %s read: %d", deviceName, percentage);
}

void SoilMoistureSensor::getSoilMoistureInterval(){
    if(millis() - lastRead < (readIntervalSeconds * 1000)){
        getSoilMoisture();
        lastRead = millis();
    }
}

class Relay: public AsyncCommDevice{
    public:
        Relay(int pin, AsyncComm * asyncComm, const char * deviceName = "relay"): AsyncCommDevice(asyncComm, deviceName), pin(pin){
            setPin(pin);
        }
        void setup_hook();
        void loop_hook();
        virtual void on();
        virtual void off();
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
        AsyncComm * asyncComm;
};

void Relay::setup_hook(){
    asyncComm->logInfo("Relay %s setup", name);
    addAction("on", (void (*)())&on);
    addAction("off", (void (*)())&off);
}

void Relay::loop_hook(){
    asyncComm->logInfo("Relay %s hook", name);
    processEvent();
}

void Relay::on(){
    asyncComm->logDebug("Relay %s set on", name);
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
    asyncComm->logDebug("Relay %s set off", name);
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
        Pump(int pin, AsyncComm * asyncComm, const char * deviceName = "pump", int troughtput_l_min = 900) : Relay(pin, asyncComm, deviceName), troughtput_l_min(troughtput_l_min) {}
        virtual void on(){
            Relay* _this = this;
            _this->on();
            pumpStartTime = millis();
            //collect statistics
        }
        virtual void off(){
            Relay* _this = this;
            _this->off();
            pumpStopTime = millis();
            estimatedWateredLiters[estimatedWateredLitersIndex] = ((pumpStopTime - pumpStartTime) / 60000) * troughtput_l_min;
            estimatedWateredLitersIndex++;
            //collect statistics
        }
        int getTroughtput(){
            return troughtput_l_min;
        }
    protected:
        int troughtput_l_min;
        unsigned long pumpStartTime;
        unsigned long pumpStopTime;
        int estimatedWateredLiters[20];
        int estimatedWateredLitersIndex;
};


class MoistureSensorPower : public Relay{
    public:
        MoistureSensorPower(int pin, AsyncComm * asyncComm, const char * deviceName = "moisturepower") : Relay(pin, asyncComm, deviceName) {}
};

class AnalogReader : public AsyncCommDevice{
    public:
        AnalogReader(int pin, AsyncComm * asyncComm, const char * deviceName = "AnalogReader") : pin(pin), AsyncCommDevice(asyncComm, deviceName) {}
        void setup_hook();
        void loop_hook(){
            processEvent();
        }
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, INPUT);
        }
        void readValueRPC();
        void readValueEvent();
    protected:
        int pin;
};

void AnalogReader::setup_hook(){
    asyncComm->logInfo("AnalogReader %s setup", deviceName);
    setPin(pin);
    addAction("readValue", (void (*)())&readValueEvent);
}

void AnalogReader::readValueRPC(){
    asyncComm->logDebug("AnalogReader %s hook", deviceName);
    int value = analogRead(pin);
    asyncComm->createRPCMessage();
    asyncComm->addRPCResult(value);
    asyncComm->sendMsg();
}

void AnalogReader::readValueEvent(){
    asyncComm->logDebug("AnalogReader %s hook", deviceName);
    int value = analogRead(pin);
    asyncComm->createEventMessage();
    asyncComm->addMessageField("deviceName", deviceName);
    asyncComm->addMessageField("value", value);
    asyncComm->sendMsg();
}

class DigitalPinOUT : public AsyncCommDevice{
    public:
        DigitalPinOUT(int pin, AsyncComm * asyncComm, const char * name = "DigitalPinOUT") : pin(pin), AsyncCommDevice(asyncComm, name) {}
        void setup_hook(){
            setPin(pin);
            addAction("high", (void (*)())&setHigh);
            addAction("low", (void (*)())&setLow);
        }
        void loop_hook(){
            processEvent();
        }
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, OUTPUT);
        }
        void setHigh(){
            digitalWrite(pin, HIGH);
            asyncComm->createEventMessage();
            asyncComm->addMessageField("deviceName", deviceName);
            asyncComm->addMessageField("state", "high");
            asyncComm->sendMsg();
        }
        void setLow(){
            digitalWrite(pin, LOW);
            asyncComm->createEventMessage();
            asyncComm->addMessageField("deviceName", deviceName);
            asyncComm->addMessageField("state", "low");
            asyncComm->sendMsg();
        }
    protected:
        int pin;
};

//end of sensors

class Waterring{
    public:
    Waterring(WaterringComm * waterringComm) : waterringComm(waterringComm), currentWaterringState(AUTOMATED) {};
    void setup_hook(){}
    void loop_hook(){}
    void manualOn(){
        currentWaterringState = MANUAL_ON;
    }
    void manualOff(){
        currentWaterringState = MANUAL_OFF;
    }
    void manualCycle(){
        currentWaterringState = MANUAL_CYCLE;
    }
    protected:
    WaterringState currentWaterringState;
    WaterringComm * waterringComm;
};

/**
 * @brief WaterringPump2MoistureSensor class
 * State machine for waterring single place 
*/
class WaterringPump2MoistureSensor : public Waterring{
    public:
        WaterringPump2MoistureSensor(WaterringComm * waterringComm, Pump* pump, SoilMoistureSensor* soilMoistureSensor1 = NULL, SoilMoistureSensor* soilMoistureSensor2 = NULL) : Waterring(waterringComm), waterringComm(waterringComm), currentWaterringState(AUTOMATED), soilMoistureSensor1(soilMoistureSensor1), soilMoistureSensor2(soilMoistureSensor2), pump(pump), waterringTimeSeconds(60){}
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
        void automatedOn(){
            currentWaterringState = AUTOMATED;
        }
        void runManualCycle(){
            currentWaterringState = MANUAL_CYCLE;
        }
        int getWaterringCycleSeconds(){
            return wateringCycleSeconds;
        }
        void setWaterringCycleSeconds(int wateringCycleSeconds){
            this->wateringCycleSeconds = wateringCycleSeconds;
        }
        void setWaterringTimeSeconds(int waterringTimeSeconds){
            this->waterringTimeSeconds = waterringTimeSeconds;
        }
        int getWaterringTimeSeconds(){
            return waterringTimeSeconds;
        }
        void waterringStateMachine();
    protected:
        unsigned long startWateringTime;
        unsigned long lastWateringTime;
        int waterringTimeSeconds;
        unsigned long wateringCycleSeconds;
        unsigned long tmpWateringCycleSeconds;
        SoilMoistureSensor *soilMoistureSensor1;
        SoilMoistureSensor *soilMoistureSensor2;
        Pump *pump;
        SensorsState sensorsState;
        int maxDaylyWateredLiters;
};

void WaterringPump2MoistureSensor::setup_hook(){
    //setup hook for all sensors and actuators
    soilMoistureSensor1->setup_hook();
    soilMoistureSensor2->setup_hook();
    pump->setup_hook();
}

void WaterringPump2MoistureSensor::loop_hook(){
    //hook all sensors and actuators
    soilMoistureSensor1->loop_hook();
    soilMoistureSensor2->loop_hook();
    pump->loop_hook();
    waterringStateMachine();
}

void WaterringPump2MoistureSensor::waterringStateMachine(){
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
            if(millis() - startWateringTime > waterringTimeSeconds * 1000){
                pump->off();
                lastWateringTime = millis();
                currentWaterringState = AUTOMATED;
            }
            break;
    }
}


#endif //WATERING_H