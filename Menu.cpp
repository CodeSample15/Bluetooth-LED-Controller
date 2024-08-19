#include "Menu.h"

Menu::Menu(u_int8_t mainMenuSize) 
{
  display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  //dynamically allocate space for the menu
  menuItems = (MenuItem*)malloc(sizeof(MenuItem) * mainMenuSize);

  //some other stuff
  currentDotNum = 0;
  selectedItem = 0;
  menuSize = mainMenuSize;
}

void Menu::init_menu() {
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

  //set some other important variables
  currentBoxY = 0;
  currentTextY = 0;
  targetBoxY = 0;
  targetTextY = 0;

  lastDotMillis = millis();
}

void Menu::updateConnectionScreen() {
  display.clearDisplay();

  display.setTextSize(2);
  display.setCursor(0, (int16_t)(SCREEN_HEIGHT/2) - 10);
  display.print("Connecting");
  for(int i=0; i<currentDotNum; i++) {
    display.print(".");
  }

  display.display();

  if(millis() - lastDotMillis > 500) {
    lastDotMillis = millis();
    currentDotNum++;
    if(currentDotNum >= 4) {
      currentDotNum = 0;
    }
  }
}

void Menu::render() 
{
  //displays menu on the screen and updates any animations
  display.clearDisplay();
  display.setTextSize(1);

  //update target and current positions
  float deltatime = float(millis() - lastRender) / 1000;

  currentBoxY += (targetBoxY - currentBoxY) * deltatime * ANIM_SPEED;
  currentTextY += (targetTextY - currentTextY) * deltatime * ANIM_SPEED;

  //if(abs(targetBoxY - currentBoxY) > 0 && current)

  for(int i=0; i<menuSize; i++) {
    display.setCursor(2, currentTextY + (i * (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER) + (MENU_BOX_BUFFER/2)));
    display.print(menuItems[i].name);
  }

  display.drawRect(0, (int16_t)currentBoxY - (MENU_BOX_BUFFER/2) + (MENU_BOX_BUFFER/2), SCREEN_WIDTH, TEXT_PIXEL_SIZE + MENU_BOX_BUFFER, SSD1306_WHITE);

  display.display();
  lastRender = millis(); //update so that animations can be properly timed
}

void Menu::Up() {
  selectedItem--; //cuz down is up

  if(selectedItem < 0) {
    selectedItem = 0;
    return;
  }

  float newBoxPos = targetBoxY - (TEXT_PIXEL_SIZE - MENU_BOX_BUFFER);

  if(newBoxPos < 0 && targetTextY < 0) {
    targetTextY += TEXT_PIXEL_SIZE - MENU_BOX_BUFFER;
  }
  else {
    targetBoxY = newBoxPos;
  }
}

void Menu::Down()  {
  selectedItem++; //cuz up is down

  if(selectedItem > menuSize) {
    selectedItem = menuSize-1;
    return;
  }

  float newBoxPos = targetBoxY + (TEXT_PIXEL_SIZE - MENU_BOX_BUFFER);

  if(newBoxPos > SCREEN_HEIGHT - (TEXT_PIXEL_SIZE - MENU_BOX_BUFFER)) {
    targetTextY -= TEXT_PIXEL_SIZE - MENU_BOX_BUFFER;
  }
  else {
    targetBoxY = newBoxPos;
  }
}

void Menu::In() {

}

void Menu::Out() {

}