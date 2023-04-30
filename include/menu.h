
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
        Menu(const KeypadLCDControl& keypad) : currentMenuItem(MenuItem::INITIAL), keypadLCDControl(keypad) {}
        void setup_hook();
        void loop_hook();
        void printMenu();
        void printMenuItem();
        void initialMenu();
    private:
        MenuItem previousMenuItem;
        MenuItem currentMenuItem;
        KeypadLCDControl keypadLCDControl;
        Wattering waterringState;
};

void Menu::setup_hook(){
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println(WATER_BOOT_TEXT);
    keypadLCDControl.setup_hook();
    initialMenu();
}

void Menu::loop_hook(){
    keypadLCDControl.loop_hook();
    ButtonPressed buttonPressed = keypadLCDControl.keyPressed();
    if(buttonPressed != ButtonPressed::NONE){
        previousMenuItem = currentMenuItem;
        keypadLCDControl.clear();
    }
    printMenuItem();
}

void Menu::initialMenu(){
    keypadLCDControl.printTextUp(WATER_BOOT_TEXT);
    keypadLCDControl.printTextDown(WATER_MODE_AUTO_TEXT);
    keypadLCDControl.printNavNext();
}

void Menu::printMenuItem(){
    keypadLCDControl.clear();
    switch(currentMenuItem){
        case MenuItem::INITIAL:
            initialMenu();
            break;
        case MenuItem::WATERRING_START:
            break;
        case MenuItem::WATERRING_STOP:
            break;
        case MenuItem::WATERRING_CYCLE:
            break;
        case MenuItem::WATERRING_TIME:
            break;
        case MenuItem::WATERRING_CYCLE_MODE:
            break;
    }
}

#endif