#ifndef MENU_H
#define MENU_H

#include "keypadlcd.h"
#include "waterring.h"
#include <Arduino.h>

enum MenuItem{
    INITIAL,
    WATERRING_START,
    WATERRING_STOP,
    WATERRING_CYCLE_RUN,
    WATERRING_CYCLE_SETUP,
    WATERRING_TIME
};

/**
 * @brief Menu class
*/

class Menu{
    public:
        Menu(KeypadLCDControl* keypad, WaterringPump2MoistureSensor* wattering) : currentMenuItem(MenuItem::INITIAL), keypadLCDControl(keypad), wattering(wattering) {}
        void setup_hook();
        void loop_hook();
        void printMenu();
        void printMenuItem();
        void initialMenuPrint();
        void initialMenuKeys();
        void waterringStartMenuPrint();
        void waterringStartMenuKeys();
        void keys();
        void navKeys();
        void waterringStopMenuPrint();
        void waterringStopMenuKeys();
        void waterringCycleRunMenuPrint();
        void waterringCycleRunMenuKeys();
        void waterringCycleMenuPrint(){
            keypadLCDControl->printTextUp(SET_WATERRING_CYCLE_TEXT);
            keypadLCDControl->getLCD().setCursor(0,1);
            keypadLCDControl->getLCD().print(wattering->getWaterringCycleSeconds());
            keypadLCDControl->getLCD().print(WATERRING_TIME_UNIT_TEXT);
        }
        void waterringCycleMenuKeys();
        void waterringTimeMenuPrint();
        void waterringTimeMenuKeys();

    private:
        MenuItem previousMenuItem;
        MenuItem currentMenuItem;
        KeypadLCDControl * keypadLCDControl;
        WaterringPump2MoistureSensor * wattering;
        unsigned long tmpWaterringCycleSeconds;
};

void Menu::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(WATER_BOOT_TEXT);
    keypadLCDControl->setup_hook();
    initialMenuPrint();
}

void Menu::loop_hook(){
    keypadLCDControl->loop_hook();
    ButtonPressed buttonPressed = keypadLCDControl->keyPressed();
    if(buttonPressed != ButtonPressed::NONE){
        keys();
    }
    printMenuItem();
    keypadLCDControl->printNavNext();
    keypadLCDControl->printNavPrev();
}

void Menu::keys(){
    switch(currentMenuItem){
        case MenuItem::INITIAL:
            initialMenuKeys();
            break;
        case MenuItem::WATERRING_CYCLE_RUN:
            waterringCycleRunMenuKeys();
            break;
        case MenuItem::WATERRING_START:
            waterringStartMenuKeys();
            break;
        case MenuItem::WATERRING_STOP:
            waterringStopMenuKeys();
            break;
        case MenuItem::WATERRING_CYCLE_SETUP:
            waterringCycleMenuKeys();
            break;
        case MenuItem::WATERRING_TIME:
            waterringTimeMenuKeys();
            break;
    }
    navKeys();
}

void Menu::navKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::LEFT){
        previousMenuItem = currentMenuItem;
        currentMenuItem = static_cast<MenuItem>((static_cast<int>(currentMenuItem) - 1) % sizeof(MenuItem));
        if(currentMenuItem == MenuItem::WATERRING_CYCLE_SETUP){
            tmpWaterringCycleSeconds = wattering->getWaterringCycleSeconds();
        }
        keypadLCDControl->clear();
    }else if(keypadLCDControl->keyPressed() == ButtonPressed::RIGHT){
        previousMenuItem = currentMenuItem;
        currentMenuItem = static_cast<MenuItem>((static_cast<int>(currentMenuItem) + 1) % sizeof(MenuItem));
        if(currentMenuItem == MenuItem::WATERRING_CYCLE_SETUP){
            tmpWaterringCycleSeconds = wattering->getWaterringCycleSeconds();
        }
        keypadLCDControl->clear();
    }
}

void Menu::waterringStartMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        wattering->manualOn();
        keypadLCDControl->printTextDown(OK_TEXT);
        delay(1000);
        //switch to stop menu
        keypadLCDControl->printTextDown(STATUS_WATERRING_ON_TEXT);
        currentMenuItem = MenuItem::WATERRING_STOP;
    }
}

void Menu::waterringStopMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        wattering->manualOff();
        keypadLCDControl->printTextDown(OK_TEXT);
    }
}

void Menu::waterringStopMenuPrint(){
    keypadLCDControl->printTextUp(ACTION_WATERRING_OFF_TEXT);
}

void Menu::initialMenuPrint(){
    keypadLCDControl->printTextUp(WATER_BOOT_TEXT);
    if(wattering->getWaterringState() == AUTOMATED || wattering->getWaterringState() == AUTOMATED_WATERING){
        keypadLCDControl->printTextDown(WATER_MODE_AUTO_TEXT);
    }else{
        keypadLCDControl->printTextDown(WATER_MODE_MANUAL_TEXT);
    }
}

void Menu::initialMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        if(wattering->getWaterringState() == AUTOMATED || wattering->getWaterringState() == AUTOMATED_WATERING){
            wattering->manualOff();
            keypadLCDControl->printTextDown(WATER_MODE_MANUAL_TEXT);
        }else{
            wattering->automatedOn();
            keypadLCDControl->printTextDown(WATER_MODE_AUTO_TEXT);
        }
    }
}

void Menu::printMenuItem(){
    keypadLCDControl->clear();
    switch(currentMenuItem){
        case MenuItem::INITIAL:
            initialMenuPrint();
            break;
        case MenuItem::WATERRING_CYCLE_RUN:
            waterringCycleRunMenuPrint();
            break;
        case MenuItem::WATERRING_START:
            waterringStartMenuPrint();
            break;
        case MenuItem::WATERRING_STOP:
            waterringStopMenuPrint();
            break;
        case MenuItem::WATERRING_CYCLE_SETUP:
            waterringCycleMenuPrint();
            break;
        case MenuItem::WATERRING_TIME:
            waterringTimeMenuPrint();
            break;
    }
}

void Menu::waterringStartMenuPrint(){
    keypadLCDControl->printTextUp(ACTION_WATERRING_ON_TEXT);
    keypadLCDControl->printTextDown(OK_TEXT);
}

void Menu::waterringCycleMenuKeys(){
    switch(keypadLCDControl->keyPressed()){
        case ButtonPressed::UP:
            tmpWaterringCycleSeconds += 1;
            break;
        case ButtonPressed::DOWN:
            tmpWaterringCycleSeconds -= 1;
            break;
        case ButtonPressed::SELECT:
            keypadLCDControl->printTextDown(OK_TEXT);
            wattering->setWaterringCycleSeconds(tmpWaterringCycleSeconds);
            delay(1000);
            currentMenuItem = MenuItem::INITIAL;
            break;
    }
}

void Menu::waterringTimeMenuPrint(){
    keypadLCDControl->printTextUp(SET_WATERRING_TIME_TEXT);
    keypadLCDControl->getLCD().setCursor(0,1);
    keypadLCDControl->getLCD().print(wattering->getWaterringTimeSeconds());
    keypadLCDControl->getLCD().print(WATERRING_TIME_UNIT_TEXT);
}

void Menu::waterringTimeMenuKeys(){
    switch(keypadLCDControl->keyPressed()){
        case ButtonPressed::UP:
            wattering->setWaterringTimeSeconds(wattering->getWaterringTimeSeconds() + 1);
            break;
        case ButtonPressed::DOWN:
            wattering->setWaterringTimeSeconds(wattering->getWaterringTimeSeconds() - 1);
            break;
        case ButtonPressed::SELECT:
            keypadLCDControl->printTextDown(OK_TEXT);
            delay(1000);
            currentMenuItem = MenuItem::INITIAL;
            break;
    }
}

void Menu::waterringCycleRunMenuPrint(){
    keypadLCDControl->printTextUp(ACTION_WATERRING_ON_TEXT);
}

void Menu::waterringCycleRunMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        wattering->runManualCycle();
        keypadLCDControl->printTextDown(OK_TEXT);
    }
}


#endif