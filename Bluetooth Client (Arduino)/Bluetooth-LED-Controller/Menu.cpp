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

  //for the connection... screen: keeps track of when the last '.' was added to the screen so there's a bit of an animation
  lastDotMillis = millis();
}

void Menu::updateConnectionScreen() {
  //does the connecting... animation by adding a '.' to the string connecting every 500 ms 
  display->clearDisplay();

  display->setTextSize(2);
  display->setCursor(0, (int16_t)(SCREEN_HEIGHT/2) - 10);
  display->print("Connecting");
  for(int i=0; i<currentDotNum; i++) {
    display->print(".");
  }

  display->display();

  if(millis() - lastDotMillis > 500) { //500ms delay
    lastDotMillis = millis();
    currentDotNum++;
    if(currentDotNum >= 4) {
      currentDotNum = 0;
    }
  }
}

void Menu::updateItemName(int index, char newName[MAX_NAME_LENGTH]) {
  if(index < 0 || index >= menuSize) return; //simple guard to make sure programmer didn't screw up

  strcpy(menuItems[index].name, newName); //really simple reassignment of the string
}

void Menu::render() 
{
  //displays menu on the screen and updates any animations
  display->clearDisplay();
  display->setTextSize(1);

  //update target and current positions
  float deltatime = float(millis() - lastRender) / 1000; //time elapsed between frames to keep animations smoother and not reliant on screen refresh rate

  //proportionally update the positions based off of how far they are from their destinations
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

  //mirror of the box code above, bur for the text (text moves when the box is about to move off screen)
  if(abs(targetTextY - currentTextY) > 0.4 && abs(textChange) < BASE_ANIM_SPEED) {
    textChange = BASE_ANIM_SPEED * (textChange < 0 ? -1 : 1);
  }
  else if(abs(textChange) < BASE_ANIM_SPEED) {
    textChange = 0;
    currentTextY = targetTextY;
  }

  //update the actual positions
  currentBoxY += boxChange;
  currentTextY += textChange;

  //print the text of the different menu items to the screen at the animated position
  for(int i=0; i<menuSize; i++) {
    display->setCursor(2, currentTextY + (i * (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER) + (MENU_BOX_BUFFER/2)));
    display->print(menuItems[i].name);
  }

  //draw the selection rectangle at its animated position
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

  //calculate new position based off of the size of the box (size of the text plus a buffer)
  float newBoxPos = targetBoxY - (TEXT_PIXEL_SIZE + MENU_BOX_BUFFER);

  //if the box is about to go off screen, change the position of the text instead
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
  //will implement at a later time (useful for submenus)
}