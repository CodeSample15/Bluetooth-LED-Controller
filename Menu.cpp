#include "Menu.h"

Menu::Menu(u_int8_t mainMenuSize, Adafruit_SSD1306* display) 
{
  Menu::display = display;

  //dynamically allocate space for the menu
  menuItems = (MenuItem*)malloc(sizeof(MenuItem) * mainMenuSize);

  //some other stuff
  currentDotNum = 0;
  selectedItem = 0;
  menuSize = mainMenuSize;
}

void Menu::init_menu() {
  //set some other important variables
  currentBoxY = 0;
  currentTextY = 0;
  targetBoxY = 0;
  targetTextY = 0;

  lastDotMillis = millis();
}

void Menu::updateConnectionScreen() {
  display->clearDisplay();

  display->setTextSize(2);
  display->setCursor(0, (int16_t)(SCREEN_HEIGHT/2) - 10);
  display->print("Connecting");
  for(int i=0; i<currentDotNum; i++) {
    display->print(".");
  }

  display->display();

  if(millis() - lastDotMillis > 500) {
    lastDotMillis = millis();
    currentDotNum++;
    if(currentDotNum >= 4) {
      currentDotNum = 0;
    }
  }
}

void Menu::updateItemName(int index, char newName[MAX_NAME_LENGTH]) {
  if(index < 0 || index >= menuSize) return;

  strcpy(menuItems[index].name, newName);;
}

void Menu::render() 
{
  //displays menu on the screen and updates any animations
  display->clearDisplay();
  display->setTextSize(1);

  //update target and current positions
  float deltatime = float(millis() - lastRender) / 1000;

  float boxChange = (targetBoxY - currentBoxY) * deltatime * ANIM_SPEED;
  float textChange = (targetTextY - currentTextY) * deltatime * ANIM_SPEED;

  //make sure the animation doesn't fall below a certain speed and just move the box into place when reaching a certain distance to prevent stutter
  if(abs(targetBoxY - currentBoxY) > 0.4 && abs(boxChange) < BASE_ANIM_SPEED) {
    boxChange = BASE_ANIM_SPEED * (boxChange < 0 ? -1 : 1);
  }
  else if(abs(boxChange) < BASE_ANIM_SPEED) {
    boxChange = 0; //move the box into place
    currentBoxY = targetBoxY;
  }

  //mirror of the box code above
  if(abs(targetTextY - currentTextY) > 0.4 && abs(textChange) < BASE_ANIM_SPEED) {
    textChange = BASE_ANIM_SPEED * (textChange < 0 ? -1 : 1);
  }
  else if(abs(textChange) < BASE_ANIM_SPEED) {
    textChange = 0;
    currentTextY = targetTextY;
  }

  currentBoxY += boxChange;
  currentTextY += textChange;

  for(int i=0; i<menuSize; i++) {
    display->setCursor(2, currentTextY + (i * (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER) + (MENU_BOX_BUFFER/2)));
    display->print(menuItems[i].name);
  }

  display->drawRect(0, (int16_t)currentBoxY - (MENU_BOX_BUFFER/2) + (MENU_BOX_BUFFER/2), SCREEN_WIDTH, TEXT_PIXEL_SIZE + MENU_BOX_BUFFER, SSD1306_WHITE);

  display->display();
  lastRender = millis(); //update so that animations can be properly timed
}

int Menu::SelectedItemIndex() {
  return selectedItem;
}

void Menu::Up() {
  selectedItem--; //cuz down is up

  if(selectedItem < 0) {
    selectedItem = 0;
    return;
  }

  float newBoxPos = targetBoxY - (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER);

  if(newBoxPos < 0 && targetTextY < 0) {
    targetTextY += TEXT_PIXEL_SIZE + MENU_BOX_BUFFER;
  }
  else {
    targetBoxY = newBoxPos;
  }
}

void Menu::Down()  {
  selectedItem++; //cuz up is down

  if(selectedItem >= menuSize) {
    selectedItem--;
    return;
  }

  float newBoxPos = targetBoxY + (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER);

  if(newBoxPos > SCREEN_HEIGHT - (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER)) {
    targetTextY -= TEXT_PIXEL_SIZE + MENU_BOX_BUFFER;
  }
  else {
    targetBoxY = newBoxPos;
  }
}

char* Menu::In() {
 return menuItems[selectedItem].name; //change to return a blank string when entering a submenu (don't feel like programming submenus right now)
}

void Menu::Out() {
  //will implement at a later time
}