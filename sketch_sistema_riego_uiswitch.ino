#include <MD_UISwitch.h>
#include <LiquidCrystal.h>

#define MENU_MAIN    0
#define MENU_SELECT  1
#define MENU_CONFIG  2
#define MENU_STATUS  4
#define MENU_REPORT  8

#define KEYPAD_PIN   A0
#define MOISTURE_SENSOR_PIN   A2

// button constants
const uint8_t BTN_RIGHT  = 2;
const uint8_t BTN_UP     = 4;
const uint8_t BTN_DOWN   = 8;
const uint8_t BTN_LEFT   = 16;
const uint8_t BTN_SELECT = 32;
const uint8_t BTN_NONE   = 0;

// -- LCD display definitions
const uint8_t LCD_ROWS = 2;
const uint8_t LCD_COLS = 16;

const uint8_t LCD_RS = 8;
const uint8_t LCD_ENA = 9;
const uint8_t LCD_D4 = 4;
const uint8_t LCD_D5 = LCD_D4+1;
const uint8_t LCD_D6 = LCD_D4+2;
const uint8_t LCD_D7 = LCD_D4+3;

LiquidCrystal lcd(LCD_RS, LCD_ENA, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
// --

// -- Control LCD Keypad switches
MD_UISwitch_Analog::uiAnalogKeys_t kt[] =
{
  {  10, 10, 'R' },  // Right
  { 130, 15, 'U' },  // Up
  { 305, 15, 'D' },  // Down
  { 475, 15, 'L' },  // Left
  { 720, 15, 'S' },  // Select
};

MD_UISwitch_Analog keyPad(KEYPAD_PIN, kt, ARRAY_SIZE(kt));
// --

void setup() {
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.clear();
  Serial.begin(9600);
  pinMode(KEYPAD_PIN, INPUT);

  keyPad.begin();
  keyPad.enableDoublePress(false);
  keyPad.enableLongPress(false);
  keyPad.enableRepeat(false);
  keyPad.enableRepeatResult(false);
}

// menu main
bool refresh = true;
int menuLevel = MENU_MAIN;

// menu select
int menuSelectPos = 0;
String menuSelectOpts[3] = { "ESTADO        > ", "CONFIGURAR    > ", "REPORTES      > " };
int menuSelectLevels[3] = { MENU_STATUS, MENU_CONFIG, MENU_REPORT };
int MENU_SELECT_SIZE = 3;

void loop() {
  
  int key = readButton();
  
  // logMenuState(key);
  readSensors();
  
  switch(menuLevel) {
    case MENU_MAIN:
      handleMainMenu(key);
      break;
    case MENU_SELECT:
      handleSelectMenu(key);
      break;
    case MENU_STATUS:
      handleStatusMenu(key);
      break;
    case MENU_CONFIG:
      handleConfigMenu(key);
      break;
    case MENU_REPORT:
      handleReportMenu(key);
      break;
    default:
      Serial.println("Unexpected menu level!");      
  }    
}

int soilHumidityPercent;
int airHumidityPercent;
int airTemperatureDegrees;
bool sensorsRefresh = false;
void readSensors() {
  if (millis() % 100 == 0) {    
    soilHumidityPercent = readHumiditySensor();
    sensorsRefresh = true;
  }
}

void handleReportMenu(int key) {
  if (refresh) {
    renderReportMenu();
    refresh = false;
  }

  if (key == BTN_LEFT) {
    menuLevel = MENU_SELECT;
    refresh = true;
  }
}

void renderReportMenu() {
  lcd.setCursor(0, 0);
  lcd.print("REPORTES      < ");
  lcd.setCursor(0, 1);
  
  lcd.print("ULT.RIEGO:");
  lcd.setCursor(11, 1);
  lcd.print("10:43");
}

void handleConfigMenu(int key) {
  if (refresh) {
    renderConfigMenu();
    refresh = false;
  }
  if (key == BTN_LEFT) {
    menuLevel = MENU_SELECT;
    refresh = true;
  }
}

void renderConfigMenu() {
  lcd.setCursor(0, 0);
  lcd.print("CONFIGURAR    < ");
  lcd.setCursor(0, 1);
  
  lcd.print("HUM.RIEGO:");
  lcd.setCursor(12, 1);
  lcd.print("60%");
}

int currentStatusMenuOption = 0;
String statusMenuOptions[] = { "HUM. SUELO:    %", "TEMPERATURA:   C", "HUM. AMBIEN:   %" };
int STATUS_MENU_SIZE = 3;

void handleStatusMenu(int key) {  

  if (refresh || sensorsRefresh) {
    renderStatusMenu();
    refresh = false;
    sensorsRefresh = false;
  }
  
  if (key == BTN_LEFT) {
    menuLevel = MENU_SELECT;
    refresh = true;
  } else if (key == BTN_DOWN) {
    if (currentStatusMenuOption < STATUS_MENU_SIZE - 1) {
      currentStatusMenuOption++;
    } else {
      currentStatusMenuOption = 0;
    }
    refresh = true;
  } else if (key == BTN_UP) {
    if (currentStatusMenuOption > 0) {
      currentStatusMenuOption--;
    } else {
      currentStatusMenuOption = STATUS_MENU_SIZE - 1;
    }
    refresh = true;
  }

}

void renderStatusMenu() {
  lcd.setCursor(0, 0);
  lcd.print("ESTADO        < ");
  lcd.setCursor(0, 1);

  lcd.print(statusMenuOptions[currentStatusMenuOption]);
  lcd.setCursor(13, 1);
  lcd.print(readStatusMenuSensor());
}

int readStatusMenuSensor() {
  switch(currentStatusMenuOption) {
    case 0:
      return soilHumidityPercent;
    case 1:
      return airTemperatureDegrees;
    case 2:
      return airHumidityPercent;
    default:
      return 0;
  }
}

void handleMainMenu(int key) {
  if (refresh) {
      renderMainMenu();
      refresh = false;
    }
    if (key != BTN_NONE) {
      menuLevel = MENU_SELECT;
      refresh = true;
    }
}

void handleSelectMenu(int key) {
  if (refresh) {
      renderMenuSelect();
      refresh = false;
    }
    if (key == BTN_DOWN) {
      if (menuSelectPos < MENU_SELECT_SIZE - 1) {
        menuSelectPos++;
      } else {
        menuSelectPos = 0;
      }
      refresh = true;
    } else if (key == BTN_UP) {
      if (menuSelectPos > 0) {
        menuSelectPos--;
      } else {
        menuSelectPos = MENU_SELECT_SIZE - 1;
      }
      refresh = true;
    } else if (key == BTN_LEFT) {
      menuLevel = MENU_MAIN;
      refresh = true;
    } else if (key == BTN_SELECT) {
      menuLevel = menuSelectLevels[menuSelectPos];
      refresh = true;
    }
}

int readButton() {

  MD_UISwitch::keyResult_t k = keyPad.read();
  if (k == MD_UISwitch::KEY_PRESS) {
    switch(keyPad.getKey()) {
      case 'U':      Serial.println("UP");      return BTN_UP;
      case 'D':      Serial.println("DOWN");    return BTN_DOWN;
      case 'L':      Serial.println("LEFT");    return BTN_LEFT;
      case 'R':      Serial.println("RIGHT");   return BTN_RIGHT;
      case 'S':      Serial.println("SELECT");  return BTN_SELECT;
      default:       Serial.println("UNKNOWN");    return BTN_NONE;
    }
  } 

  return BTN_NONE;
  
}

int readHumiditySensor() {
  int sensorValue=analogRead(MOISTURE_SENSOR_PIN);
  
  int percentValue=map(sensorValue, 1023, 200, 0, 100);
  Serial.print("Soil Moisture: ");
  Serial.print(percentValue);
  Serial.println(" %");

  return percentValue;
}

void logMenuState(int key) {
  /*
  Serial.print(millis());
  Serial.print("- key: ");
  Serial.print(key);
  Serial.print(" - menu lvl:");
  Serial.print(menuLevel);
  Serial.print(" - menu opt:");
  Serial.println(menuSelectPos);  
  */
}

void renderMenuSelect() {
  lcd.setCursor(0, 0);
  lcd.print("MENU          < ");
  lcd.setCursor(0, 1);
  lcd.print(menuSelectOpts[menuSelectPos]);
}

void renderMainMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA DE RIEGO");
  lcd.setCursor(12, 1);
  lcd.print("V1.0");
}
