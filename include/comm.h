#ifndef COMM_H
#define COMM_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include "logger.h"
#include "async_comm.h"

class WaterringComm : public AsyncComm {
    WaterringComm(Stream* io, DynamicJsonDocument* doc, Logger* _logger, const char * deviceName = "unknown"): AsyncComm(io, doc, deviceName){
        this->setLogger(_logger);
    }
    virtual void processLogger();
    virtual void processEvent();
};

void WaterringComm::processLogger(){
    logger->info("Processing log");
}

void WaterringComm::processEvent(){
    logger->info("Processing event");
}

#endif // COMM_H