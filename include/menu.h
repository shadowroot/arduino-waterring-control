
#ifndef MENU_H
#define MENU_H
#include "keypadlcd.h"
#include "wattering.h"
#include <Arduino.h>

enum MenuItem{
    INITIAL,
    WATERRING_START,
    WATERRING_STOP,
    WATERRING_CYCLE,
    WATERRING_TIME,
    WATERRING_CYCLE_MODE,
};

/**
 * @brief Menu class
*/

class Menu{
    public:
        Menu(KeypadLCDControl* keypad, Waterring* waterringState) : currentMenuItem(MenuItem::INITIAL), keypadLCDControl(keypad), waterringState(waterringState) {}
        void setup_hook();
        void loop_hook();
        void printMenu();
        void printMenuItem();
        void initialMenu();
        void waterringStartMenuPrint();
        void waterringStartMenuKeys();
        void keys();
        void navKeys();
        void waterringStopMenuPrint();
        void waterringStopMenuKeys();
        void waterringCycleMenuPrint(){
            keypadLCDControl->printTextUp(SET_WATERRING_CYCLE_TEXT);
            keypadLCDControl->getLCD().setCursor(0,1);
            keypadLCDControl->getLCD().print(waterringState->getWatteringCycleSeconds());
            keypadLCDControl->getLCD().print(WATERRING_TIME_UNIT_TEXT);
        }
        void waterringCycleMenuKeys();
        void waterringTimeMenuPrint();
        void waterringCycleModeMenuPrint();
        void waterringTimeMenuKeys();
        void waterringCycleModeMenuKeys();

    private:
        MenuItem previousMenuItem;
        MenuItem currentMenuItem;
        KeypadLCDControl * keypadLCDControl;
        Waterring * waterringState;
};

void Menu::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(WATER_BOOT_TEXT);
    keypadLCDControl->setup_hook();
    initialMenu();
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
            break;
        case MenuItem::WATERRING_START:
            waterringStartMenuKeys();
            break;
        case MenuItem::WATERRING_STOP:
            waterringStopMenuKeys();
            break;
        case MenuItem::WATERRING_CYCLE:
            break;
        case MenuItem::WATERRING_TIME:
            break;
        case MenuItem::WATERRING_CYCLE_MODE:
            break;
    }
    navKeys();
}

void Menu::navKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::LEFT){
        previousMenuItem = currentMenuItem;
        currentMenuItem = static_cast<MenuItem>((static_cast<int>(currentMenuItem) - 1) % sizeof(MenuItem));
        keypadLCDControl->clear();
    }else if(keypadLCDControl->keyPressed() == ButtonPressed::RIGHT){
        previousMenuItem = currentMenuItem;
        currentMenuItem = static_cast<MenuItem>((static_cast<int>(currentMenuItem) + 1) % sizeof(MenuItem));
        keypadLCDControl->clear();
    }
}

void Menu::waterringStartMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        waterringState->manualOn();
        keypadLCDControl->printTextDown(OK_TEXT);
        delay(1000);
        //switch to stop menu
        keypadLCDControl->printTextDown(STATUS_WATERRING_ON_TEXT);
        currentMenuItem = MenuItem::WATERRING_STOP;
    }
}

void Menu::waterringStopMenuKeys(){
    if(keypadLCDControl->keyPressed() == ButtonPressed::SELECT){
        waterringState->manualOff();
        keypadLCDControl->printTextDown(OK_TEXT);
    }
}

void Menu::waterringStopMenuPrint(){
    keypadLCDControl->printTextUp(ACTION_WATERRING_OFF_TEXT);
}

void Menu::initialMenu(){
    keypadLCDControl->printTextUp(WATER_BOOT_TEXT);
    if(waterringState->getWatteringState() == AUTOMATED || waterringState->getWatteringState() == AUTOMATED_WATERING){
        keypadLCDControl->printTextDown(WATER_MODE_AUTO_TEXT);
    }else{
        keypadLCDControl->printTextDown(WATER_MODE_MANUAL_TEXT);
    }
}

void Menu::printMenuItem(){
    keypadLCDControl->clear();
    switch(currentMenuItem){
        case MenuItem::INITIAL:
            initialMenu();
            break;
        case MenuItem::WATERRING_START:
            waterringStartMenuPrint();
            break;
        case MenuItem::WATERRING_STOP:
            waterringStopMenuPrint();
            break;
        case MenuItem::WATERRING_CYCLE:
            break;
        case MenuItem::WATERRING_TIME:
            break;
        case MenuItem::WATERRING_CYCLE_MODE:
            break;
    }
}

void Menu::waterringStartMenuPrint(){
    keypadLCDControl->printTextUp(ACTION_WATERRING_ON_TEXT);
    keypadLCDControl->printTextDown(OK_TEXT);
}

#endif