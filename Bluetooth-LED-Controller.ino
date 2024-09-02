#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h> //imu
#include "Menu.h"

/*
  Light modes:
  0 - off
  1 - static (set based off of raw, pitch, and roll characteristics for rgb)
  2 - waves
  3 - stars
  4 - rainbow
*/

//For the screen
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SCREEN_CENTER_X 64
#define SCREEN_CENTER_Y 32

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

//Uuids
#define deviceServiceUuid "f6351508-a501-4022-acba-7232431a7fca"
#define deviceServiceYawCharacteristicUuid "57ead45a-9ac2-4810-98c3-63191c97cece"
#define deviceServicePitchCharacteristicUuid "0c712b41-f2ec-462d-b60e-5defdc405db2"
#define deviceServiceRollCharacteristicUuid "c4b5dc3e-0bcd-4f34-93be-3afbabc6eed8"
#define deviceServiceModeCharacteristicUuid "4722c037-0957-4d33-b8a4-69378fd2ef9a"

#define NUM_MAIN_MENU_ITEMS 3

#define SLEEP_TIME_MILLIS 15000 //time until the device goes to sleep and turns off the screen
#define SLEEP_GYRO_MAX 10 //threshold gyro readings need to pass until the device is turned back on

//for gyro control
#define CONTROL_SENSITIVITY_Y 100 //how far the gyro has to move for a input to be registered
#define CONTROL_SENSITIVITY_X 200
#define CONTROL_TIME 500 //how long in millis it should take for a gyro change (negative to positive or vice versa) to be registered. Any longer and the gyro movement will be considred random

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float yaw, pitch, roll;
Menu mainMenu(NUM_MAIN_MENU_ITEMS, &display);
Menu patternMenu(4, &display);
Menu colorMenu(8, &display);

int lastChangeX, lastChangeY, lastChangeA; // last time in millis that the gyro reading went from positive to negative (only respond to quick rapid movements)
bool positiveChangeX, positiveChangeY, positiveChangeA; //A is for for accelerometer readings

long lastGyroMoveMillis;
bool awake;

char currentScreen[MAX_NAME_LENGTH];
char currentPattern[MAX_NAME_LENGTH];
char currentColor[MAX_NAME_LENGTH];

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay(); //get rid of the cringe adafruit logo (jk i love adafruit)
  display.display(); //clear the display

  //set text settings
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  //start modules
  if(!BLE.begin()) {
    Serial.println("Failed to start BLE module!");
    while(1);
  }

  if(!IMU.begin()) {
    Serial.println("Failed to start IMU module!");
    while(1);
  }

  //init menu and display
  init_menus();

  strcpy(currentScreen, "");

  Serial.println("BLE module started...");
  BLE.setDeviceName("BluetoothLEDController");
  BLE.advertise();
}

void loop() {
  connectToPeripheral();
}

void connectToPeripheral() {
  BLEDevice peripheral;

  Serial.println("Scanning for device...");

  do {
    BLE.scanForUuid(deviceServiceUuid);
    peripheral = BLE.available();

    mainMenu.updateConnectionScreen(); //draw the Connecting... thing on the screen
    delay(100);
  } while(!peripheral);

  Serial.print("Found device!\t:");
  Serial.println(peripheral.localName());
  BLE.stopScan();
  controlPeripheral(peripheral);
}

void controlPeripheral(BLEDevice peripheral) {
  if(peripheral.connect()) {
    Serial.println("Connected to device!");
    Serial.println("");
  }
  else {
    Serial.println("Failed to connect to device...");
    Serial.println("");
    return;
  }

  if(peripheral.discoverAttributes()) {
    Serial.println("Peripheral device attributes discovered!");
    Serial.println("");
  }
  else {
    Serial.println("Peripheral device attributes discovery failed!");
    Serial.println("");
    peripheral.disconnect();
    return;
  }

  BLECharacteristic yawCharacteristic = peripheral.characteristic(deviceServiceYawCharacteristicUuid);
  BLECharacteristic pitchCharacteristic = peripheral.characteristic(deviceServicePitchCharacteristicUuid);
  BLECharacteristic rollCharacteristic = peripheral.characteristic(deviceServiceRollCharacteristicUuid);
  BLECharacteristic modeCharacteristic = peripheral.characteristic(deviceServiceModeCharacteristicUuid);

  //check to make sure characteristics are discoverable (prevent errors later in the main loop)
  if(!CheckCharacteristic(yawCharacteristic, peripheral)) return;
  if(!CheckCharacteristic(pitchCharacteristic, peripheral)) return;
  if(!CheckCharacteristic(rollCharacteristic, peripheral)) return;
  if(!CheckCharacteristic(modeCharacteristic, peripheral)) return;

  awake = true;

  while(peripheral.connected()) {
    if(IMU.gyroscopeAvailable() && IMU.accelerationAvailable()) {
      IMU.readGyroscope(yaw, pitch, roll);

      if(yaw >= SLEEP_GYRO_MAX || pitch >= SLEEP_GYRO_MAX || roll >= SLEEP_GYRO_MAX) {
        lastGyroMoveMillis = millis();
        awake = true;
      }
      else if(millis() - lastGyroMoveMillis > SLEEP_TIME_MILLIS) {
        awake = false; //nighty night
      }
    }

    if(awake) {
      if(strcmp(currentScreen, "") == 0) {
        controlMenu(yaw, pitch);
      }
      else {
        controlPage(yaw, pitch, yawCharacteristic, pitchCharacteristic, rollCharacteristic, modeCharacteristic); //to keep the messy code for all of the pages at the bottom of this file
      }
    }
    else {
      //go to sleep
      display.clearDisplay();
      display.display();
    }
  }

  Serial.println("Peripheral device disconnected!");
  Serial.println("");
}

bool CheckCharacteristic(BLECharacteristic characteristic, BLEDevice peripheral) {
  if(!characteristic) {
    Serial.println("Connected device does not have a characteristic!");
    peripheral.disconnect();
    return false;
  } else if(!characteristic.canWrite()) {
    Serial.println("Connected device has a characteristic that is non writable!");
    peripheral.disconnect();
    return false;
  }

  return true;
}

void init_menus() {
  //number of menu items MUST be the same as NUM_MAIN_MENU_ITEMS
  mainMenu.init_menu();

  mainMenu.updateItemName(0, "Select Pattern");
  mainMenu.updateItemName(1, "Strip Color");
  mainMenu.updateItemName(2, "Off");

  patternMenu.init_menu();

  patternMenu.updateItemName(0, "Off");
  patternMenu.updateItemName(1, "Waves");
  patternMenu.updateItemName(2, "Stars");
  patternMenu.updateItemName(3, "Rainbow");

  colorMenu.init_menu();

  colorMenu.updateItemName(0, "White");
  colorMenu.updateItemName(1, "Red");
  colorMenu.updateItemName(2, "Green");
  colorMenu.updateItemName(3, "Blue");
  colorMenu.updateItemName(4, "Yellow");
  colorMenu.updateItemName(5, "Pink");
  colorMenu.updateItemName(6, "Cyan");
  colorMenu.updateItemName(7, "Dark Blue");
}

int getGyroControlInput(float x, float y) 
{
  int control = 0;

  if(abs(x) >= CONTROL_SENSITIVITY_X) {
    if(millis() - lastChangeX <= CONTROL_TIME) {
      if(x > 0 && !positiveChangeX)
        control = 1;
      else if(x < 0 && positiveChangeX)
        control = 2;
    }

    positiveChangeX = x > 0;
    lastChangeX = millis();
  }

  if(abs(y) >= CONTROL_SENSITIVITY_Y) {
    if(millis() - lastChangeY <= CONTROL_TIME) {
      if(y > 0 && !positiveChangeY)
        control = 3;
      else if(y < 0 && positiveChangeY)
        control = 4;
    }

    positiveChangeY = y > 0;
    lastChangeY = millis();
  }

  return control;
}

void controlMenu(float yaw, float pitch) {
  //handle the graphical side of things
  mainMenu.render();

  //handle the control of the menu via gyro
  switch(getGyroControlInput(yaw, pitch))
  {
    case 1:
      mainMenu.Up();
      break;
    
    case 2:
      mainMenu.Down();
      break;

    case 3:
      strcpy(currentScreen, mainMenu.In());
      break;

    case 4:
      mainMenu.Out();
      break;
  }
}

void controlPage(float yaw, float pitch, BLECharacteristic yawChar, BLECharacteristic pitchChar, BLECharacteristic rollChar, BLECharacteristic modeChar) {
  int control = getGyroControlInput(yaw, pitch);

  if(strcmp(currentScreen, "Off") == 0) {
    modeChar.writeValue((int8_t)0);
    strcpy(currentScreen, "");
  }
  else if(strcmp(currentScreen, "Strip Color") == 0) {
    strcpy(currentScreen, ""); //the main code on the raspi is fucking broken for this mode >:(

    colorMenu.render();

    int8_t r = 0;
    int8_t g = 0;
    int8_t b = 0;

    switch(control) 
    {
      case 1:
        colorMenu.Up();
        break;
      
      case 2:
        colorMenu.Down();
        break;

      case 3:
        strcpy(currentColor, colorMenu.In());
        modeChar.writeValue((int8_t)1); //tell the device it's in static color mode

        switch(colorMenu.SelectedItemIndex())
        {
          case 0:
            r = 255;
            g = 255;
            b = 255;
            break; //white

          case 1:
            r = 255;
            g = 0;
            b = 0;
            break; //red

          case 2:
            r = 0;
            g = 255;
            b = 0;
            break; //green

          case 3:
            r = 0;
            g = 0;
            b = 255;
            break; //blue

          case 4:
            r = 250;
            g = 180;
            b = 0;
            break; //yellow

          case 5:
            r = 255;
            g = 0;
            b = 255;
            break; //pink

          case 6:
            r = 0;
            g = 150;
            b = 170;
            break; //cyan

          case 7:
            r = 0;
            g = 10;
            b = 120;
            break; //dark blue
        }

        yawChar.writeValue(r);
        pitchChar.writeValue(g);
        rollChar.writeValue(b);
        break;

      case 4:
        strcpy(currentScreen, ""); //exit to main screen
        break;
    }
  }
  else if(strcmp(currentScreen, "Select Pattern") == 0) {
    patternMenu.render();

    switch(control)
    {
      case 1:
        patternMenu.Up();
        break;
      
      case 2:
        patternMenu.Down();
        break;

      case 3:
        strcpy(currentPattern, patternMenu.In());
        //update the bluetooth server
        if(strcmp(currentPattern, "Off") == 0)
          modeChar.writeValue((int8_t)0);
        else if(strcmp(currentPattern, "Waves") == 0)
          modeChar.writeValue((int8_t)2);
        else if(strcmp(currentPattern, "Stars") == 0)
          modeChar.writeValue((int8_t)3);
        else if(strcmp(currentPattern, "Rainbow") == 0)
          modeChar.writeValue((int8_t)4);
        break;

      case 4:
        strcpy(currentScreen, ""); //exit to main screen
        break;
    }
  }
}
