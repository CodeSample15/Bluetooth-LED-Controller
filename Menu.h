#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SCREEN_CENTER_X 64
#define SCREEN_CENTER_Y 32

#define TEXT_PIXEL_SIZE 7 //both of these values are pixel counts
#define MENU_BOX_BUFFER 4

#define MAX_NAME_LENGTH 40

#define ANIM_SPEED 130
#define BASE_ANIM_SPEED 0.2

typedef struct MenuItem {
    char name[MAX_NAME_LENGTH];
} MenuItem;

class Menu  {
  public:
    Menu(u_int8_t mainMenuSize, Adafruit_SSD1306* display);

    void init_menu();

    void Up();
    void Down();
    char* In();
    void Out();

    void updateConnectionScreen();
    void updateItemName(int index, char newName[MAX_NAME_LENGTH]);

    void render();

    int SelectedItemIndex();
    
    char currentScreen[MAX_NAME_LENGTH];
    MenuItem* menuItems;
    Adafruit_SSD1306* display;
  private:
    int currentOffset; //current offset for rendering the text of the menu
    int menuSize;

    int selectedItem;

    //for the connection screen
    int currentDotNum;
    int lastDotMillis;

    //for rendering the menu and keeping a pretty UI
    float currentBoxY;
    float currentTextY;
    float targetBoxY;
    float targetTextY;

    int lastRender; //to calculate dtime for animations and shit
};