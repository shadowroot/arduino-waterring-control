#ifndef WATERRING_H
#define WATERRING_H

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
    GOOD
};

class BaseSensor{
    public:
        BaseSensor(const char * deviceName = "BaseSensor"){}
    protected:
        const char * deviceName;
};

//start of sensor and actuators

class SoilMoistureSensor: public BaseSensor{
    public:
        SoilMoistureSensor(int pin, const char * deviceName = "Soil_moisture_sensor", int readIntervalSeconds=1800, int treashold=40) : BaseSensor(deviceName), pin(pin), readIntervalSeconds(readIntervalSeconds), lastRead(0), treashold(treashold) {}
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
}

void SoilMoistureSensor::loop_hook(){
    getSoilMoistureInterval();
}

void SoilMoistureSensor::getSoilMoisture(){
    int analogValue = analogRead(pin);
    percentage = map(analogValue, 0, 1023, 0, 100);
    Serial.printf("Soil moisture %s: %d", deviceName, percentage);
}

void SoilMoistureSensor::getSoilMoistureInterval(){
    if(millis() - lastRead < (readIntervalSeconds * 1000)){
        getSoilMoisture();
        lastRead = millis();
    }
}

class Relay{
    public:
        Relay(int pin, const char * deviceName = "relay"): pin(pin){
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
};

void Relay::setup_hook(){
    Serial.printf("Relay %s setup", name);
}

void Relay::loop_hook(){
    Serial.printf("Relay %s hook", name);
}

void Relay::on(){
    Serial.printf("Relay %s set on", name);
    digitalWrite(pin, HIGH);
    lastRunning = millis();
    running = true;
}

void Relay::off(){
    Serial.printf("Relay %s set off", name);
    digitalWrite(pin, LOW);
    runningTime = millis() - lastRunning;
    running = false;
}


class Pump : public Relay{
    public:
        Pump(int pin, const char * deviceName = "pump", int troughtput_l_min = 900) : Relay(pin, deviceName), troughtput_l_min(troughtput_l_min) {}
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
        MoistureSensorPower(int pin, const char * deviceName = "moisturepower") : Relay(pin, deviceName) {}
};


class DigitalPinOUT : public BaseSensor{
    public:
        DigitalPinOUT(int pin, const char * name = "DigitalPinOUT") : BaseSensor(deviceName), pin(pin){}
        void setup_hook(){
            setPin(pin);
        }
        void loop_hook(){
        }
        void setPin(int pin){
            this->pin = pin;
            pinMode(pin, OUTPUT);
        }
        void setHigh(){
            digitalWrite(pin, HIGH);
        }
        void setLow(){
            digitalWrite(pin, LOW);
        }
    protected:
        int pin;
};

//end of sensors

class Waterring{
    public:
    Waterring() : currentWaterringState(WaterringState::AUTOMATED) {};
    void setup_hook(){}
    void loop_hook(){}
    void manualOn(){
        currentWaterringState = WaterringState::MANUAL_ON;
    }
    void manualOff(){
        currentWaterringState = WaterringState::MANUAL_OFF;
    }
    void manualCycle(){
        currentWaterringState = WaterringState::MANUAL_CYCLE;
    }
    protected:
    WaterringState currentWaterringState;
};

/**
 * @brief WaterringPump2MoistureSensor class
 * State machine for waterring single place 
*/
class WaterringPump2MoistureSensor : public Waterring{
    public:
        WaterringPump2MoistureSensor(Pump* pump, SoilMoistureSensor* soilMoistureSensor1 = NULL, SoilMoistureSensor* soilMoistureSensor2 = NULL) : Waterring(), soilMoistureSensor1(soilMoistureSensor1), soilMoistureSensor2(soilMoistureSensor2), pump(pump), waterringTimeSeconds(60){}
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
    Serial.println("WaterringPump2MoistureSensor::loop_hook");
    Serial.println("MoistureSensor1 loop_hook");
    soilMoistureSensor1->loop_hook();
    Serial.println("MoistureSensor2 loop_hook");
    soilMoistureSensor2->loop_hook();
    Serial.println("Pump loop_hook");
    pump->loop_hook();
    Serial.println("Waterring state machine");
    waterringStateMachine();
}

void WaterringPump2MoistureSensor::waterringStateMachine(){
    switch(currentWaterringState){
        case AUTOMATED_WATERING:
            if(millis() - startWateringTime > wateringCycleSeconds * 1000){
                currentWaterringState = WaterringState::AUTOMATED;
                pump->off();
                //either not working or disconnected
                if((soilMoistureSensor1 && soilMoistureSensor1->isError()) && (soilMoistureSensor2 && soilMoistureSensor2->isError())){
                    sensorsState = SensorsState::FAIL_ALL_SENSORS;
                }
                else if(soilMoistureSensor1 && soilMoistureSensor1->isError()){
                    sensorsState = SensorsState::FAIL_SENSOR1;
                }
                else if(soilMoistureSensor2 && soilMoistureSensor2->isError()){
                    sensorsState = SensorsState::FAIL_SENSOR2;
                }
                else{
                    sensorsState = SensorsState::GOOD;
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