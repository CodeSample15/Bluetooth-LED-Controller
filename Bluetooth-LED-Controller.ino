#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h> //imu
#include "Menu.h"

/*
  Light modes:
  0 - off
  1 - static (set based off of raw, pitch, and roll characteristics)
  2 - waves
  3 - stars
  4 - rainbow
*/

//Uuids
#define deviceServiceUuid "f6351508-a501-4022-acba-7232431a7fca"
#define deviceServiceYawCharacteristicUuid "57ead45a-9ac2-4810-98c3-63191c97cece"
#define deviceServicePitchCharacteristicUuid "0c712b41-f2ec-462d-b60e-5defdc405db2"
#define deviceServiceRollCharacteristicUuid "c4b5dc3e-0bcd-4f34-93be-3afbabc6eed8"
#define deviceServiceModeCharacteristicUuid "4722c037-0957-4d33-b8a4-69378fd2ef9a"

#define NUM_MENU_ITEMS 4

//for gyro control
#define CONTROL_SENSITIVITY 300 //how far the gyro has to move for a input to be registered
#define CONTROL_TIME 500 //how long in millis it should take for a gyro change (negative to positive or vice versa) to be registered. Any longer and the gyro movement will be considred random

float yaw, pitch, roll;
Menu screenMenu(NUM_MENU_ITEMS);

int lastChangeX, lastChangeY; // last time in millis that the gyro reading went from positive to negative (only respond to quick rapid movements)
bool positiveChangeX, positiveChangeY;

char* currentScreen;

void setup() {
  Serial.begin(9600);

  if(!BLE.begin()) {
    Serial.println("Failed to start BLE module!");
    while(1);
  }

  if(!IMU.begin()) {
    Serial.println("Failed to start IMU module!");
    while(1);
  }

  //init menu and display
  init_menu();

  currentScreen = "";

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

    screenMenu.updateConnectionScreen(); //draw the Connecting... thing on the screen
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

  float lastYaw = 0;
  float lastPitch = 0;
  float lastRoll = 0;

  while(peripheral.connected()) {
    if(IMU.gyroscopeAvailable() && IMU.accelerationAvailable()) {
      IMU.readGyroscope(yaw, pitch, roll);
    }

    //handle the graphical side of things
    screenMenu.render();

    if(strcmp(currentScreen, "") == 0) {
      controlMenu(yaw, pitch);
    }
    else {
      controlPage(yaw, pitch); //to keep the messy code for all of the pages at the bottom of this file
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

void init_menu() {
  //number of menu items MUST be the same as NUM_MAIN_MENU_ITEMS
  screenMenu.init_menu();

  screenMenu.menuItems[0].name = "Off";
  screenMenu.menuItems[1].name = "Strip Color";
  screenMenu.menuItems[2].name = "Select Pattern";
  screenMenu.menuItems[3].name = "Gyro Control";
}

int getGyroControlInput(float x, float y) 
{
  int control = 0;

  if(abs(x) >= CONTROL_SENSITIVITY) {
    if(millis() - lastChangeX <= CONTROL_TIME) {
      if(x > 0 && !positiveChangeX)
        control = 1;
      else if(x < 0 && positiveChangeX)
        control = 2;
    }

    positiveChangeX = x > 0;
    lastChangeX = millis();
  }

  if(abs(y) >= CONTROL_SENSITIVITY) {
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
  //handle the control of the menu via gyro
    switch(getGyroControlInput(yaw, pitch))
    {
      case 1:
        screenMenu.Up();
        break;
      
      case 2:
        screenMenu.Down();
        break;

      case 3:
        currentScreen = currentscreenMenu.In();
        break;

      case 4:
        screenMenu.Out();
        break;
    }
}

void controlPage(float yaw, float pitch) {
  int control = getGyroControlInput(yaw, pitch);

  if(strcmp(currentPage, "Off")) {
    
  }
}
