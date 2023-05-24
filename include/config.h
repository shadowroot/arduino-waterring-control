

#define SOIL_MOISTURE_SENSOR1_PIN GPIO_NUM_0
#define SOIL_MOISTURE_SENSOR2_PIN GPIO_NUM_1
#define PUMP_PIN GPIO_NUM_2
#define MOISTURE_SENSOR_POWER_PIN GPIO_NUM_3
#define LCD_RS_PIN GPIO_NUM_10
#define LCD_EN_PIN GPIO_NUM_5
#define LCD_D4_PIN GPIO_NUM_6
#define LCD_D5_PIN GPIO_NUM_7
#define LCD_D6_PIN GPIO_NUM_8
#define LCD_D7_PIN GPIO_NUM_9
#define BUTTON_PIN GPIO_NUM_4

#define BROKER_ADDR     IPAddress(37, 205, 14, 122)
#define BROKER_PORT     1883
#define BROKER_USERNAME "haas"
#define BROKER_PASSWORD "mqtt"

#define WIFI_SSID       "Hells Gate IOT"
#define WIFI_PASSWORD   "HadejHeslo0" 


#define WATER_BOOT_TEXT "Zalevani 0.1"
#define WATER_MODE_AUTO_TEXT "Automaticke"
#define WATER_MODE_MANUAL_TEXT "Rucni"
#define ACTION_WATERRING_ON_TEXT "Spust rucni"
#define ACTION_WATERRING_ON_TEXT "Spust cyklus"
#define ACTION_WATERRING_OFF_TEXT "Zastav zalevani"
#define STATUS_WATERRING_ON_TEXT "Zalevam..."
#define SET_WATERRING_CYCLE_TEXT "Nastav interval"
#define SET_WATERRING_TIME_TEXT "Nastav dobu zalevani"
#define LAST_WATERRING_TEXT "Posledni zalevani"
#define SOIL_MOISTURE_TEXT "Vlhkost pudy"
#define WATERRING_TIME_UNIT_TEXT "s"
#define OK_TEXT "OK!"
#define RIGHT_SYMBOL ">"
#define LEFT_SYMBOL "<"
#define LCD_ROWS 2
#define LCD_COLS 16

#define OPTIMAL_SOIL_MOISTURE_PERCENT 60
