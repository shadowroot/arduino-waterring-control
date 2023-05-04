#ifndef COMM_H
#define COMM_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include "async_comm.h"

class WaterringComm : public AsyncComm {
    public:
        WaterringComm(Stream* io, DynamicJsonDocument* doc,  const char * deviceName = "unknown"): AsyncComm(io, doc, deviceName){
        }
        virtual void processLogger();
        virtual void processEvent();
};

void WaterringComm::processLogger(){
    logInfo("Processing log");
}

void WaterringComm::processEvent(){
    logInfo("Processing event");
}

#endif // COMM_H