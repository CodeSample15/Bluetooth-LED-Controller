#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define SCREEN_CENTER_X 64
#define SCREEN_CENTER_Y 32

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

#define TEXT_PIXEL_SIZE 7 //both of these values are pixel counts
#define MENU_BOX_BUFFER 4

#define MAX_NAME_LENGTH 40

#define ANIM_SPEED 130
#define BASE_ANIM_SPEED 0.2

typedef struct MenuItem {
    char* name;
} MenuItem;

class Menu  {
  public:
    Menu(u_int8_t mainMenuSize);

    void init_menu();

    void Up();
    void Down();
    void In();
    void Out();

    void updateConnectionScreen();

    void render();

    char* currentScreen;
    MenuItem* menuItems;
    Adafruit_SSD1306 display;
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