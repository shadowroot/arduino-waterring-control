#ifndef COMM_H
#define COMM_H


#include <Arduino.h>
#include <ArduinoJson.h>
#include "log.h"
#include "async_comm.h"

class WaterringComm : public AsyncComm {
    WaterringComm(Stream* io, DynamicJsonDocument* doc, Log* log): AsyncComm(io, doc, log){}
    virtual void processLog();
    virtual void processEvent();
};

void WaterringComm::processLog(){
    log->info("Processing log");
}

void WaterringComm::processEvent(){
    log->info("Processing event");
}

#endif // COMM_H