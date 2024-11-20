#if 1

#include <Adafruit_GFX.h> //library used for drawing shapes 
#include <MCUFRIEND_kbv.h> //library used for the screen 
MCUFRIEND_kbv tft;
#include <TouchScreen.h> //library for touch screen idk if we need it

//defines max and min pressure for pressing (again idk if it is needed) 
#define MINPRESSURE 200 
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button on_btn, off_btn;

int pixel_x, pixel_y; //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
TSPoint p = ts.getPoint();
pinMode(YP, OUTPUT); //restore shared pins
pinMode(XM, OUTPUT);
digitalWrite(YP, HIGH); //because TFT control pins
digitalWrite(XM, HIGH);
bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
if (pressed) {
pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
}
return pressed;
}

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//joystick
int xPin = A8;
int yPin= A9;
int buttonPin = 23;
int xVal;  // how far is x axis
int yVal; //how far is y axis
int buttonState; //is the button pushed (is this for staring the game?)

//speaker
const int SPEAKER= 29;

void setup(void)
{
Serial.begin(9600);
uint16_t ID = tft.readID();
Serial.print("TFT ID = 0x");
Serial.println(ID, HEX);
Serial.println("Calibrate for your Touch Panel");
if (ID == 0xD3D3) ID = 0x9486; // write-only shield
tft.begin(ID);
tft.setRotation(0); //PORTRAIT
tft.fillScreen(BLACK);
on_btn.initButton(&tft, 60, 200, 100, 40, WHITE, CYAN, BLACK, "ON", 2);
off_btn.initButton(&tft, 180, 200, 100, 40, WHITE, CYAN, BLACK, "OFF", 2);
on_btn.drawButton(false);
off_btn.drawButton(false);
tft.fillRect(40, 80, 160, 80, RED);
}

/* two buttons are quite simple
*/
void loop(void)
{
bool down = Touch_getXY();
on_btn.press(down && on_btn.contains(pixel_x, pixel_y));
off_btn.press(down && off_btn.contains(pixel_x, pixel_y));
if (on_btn.justReleased())
on_btn.drawButton();
if (off_btn.justReleased())
off_btn.drawButton();
if (on_btn.justPressed()) {
on_btn.drawButton(true);
tft.fillRect(40, 80, 160, 80, GREEN);
}
if (off_btn.justPressed()) {
off_btn.drawButton(true);
tft.fillRect(40, 80, 160, 80, RED);
}
}
#endif

// drawing the maze for the game:

#define ROWS 8
#define COLS 12
#define CELL_SIZE 40

// 0==space(nothing) 1=wall  2== pellet

int maze[ROWS][COLS] = {
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
  {1, 2, 2, 0, 1, 1, 0, 0, 2, 2, 2, 1},
  {1, 2, 1, 0, 1, 1, 0, 1, 1, 1, 2, 1},
  {1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 2, 1},
  {1, 0, 0, 0, 1, 1, 0, 1, 0, 0, 2, 1},
  {1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1},
  {1, 2, 2, 0, 2, 2, 0, 2, 2, 2, 2, 1},
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


void drawMaze() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x = col * CELL_SIZE;
            int y = row * CELL_SIZE;

            if (maze[row][col] == 1) {
                // Draw a wall (blue rectangle)
                TFT_fillRect(x, y, CELL_SIZE, CELL_SIZE, BLUE);
            } else if (maze[row][col] == 2) {
                // Draw a pellet (white circle)
                TFT_fillCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 5, WHITE); //since it should be placed in the center it is cellsize/2 , the 5 shows radius, we can change that based on how big it should be. 
            } else {
                // Draw an empty space (black rectangle)
                TFT_fillRect(x, y, CELL_SIZE, CELL_SIZE, BLACK);
            }
        }
    }
}

//drawing the pacman 
int pacManX = 0;
int pacManY= 0 ; 

//Drawing the man obviously 
void drawPacman(){
  TFT_fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , 10 , YELLOW);
}

//clearing the place where the man has been visually 
void clearOut(){
  TFT_fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , CELL_SIZE , BLACK);

}
 



//tracking score
 void trackingScores(){
  int score= 0 ;
  int row = pacManX/CELL_SIZE;
  int col = pacManY/CELL_SIZE;

  if(maze[row][col]==2){
    maze[row][col]=0; // updating the maze  
    TFT_fillRect(row*CELL_SIZE, col*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);//updating the visuals
    score++;
  }
 }


 //things we need to do :
 //Moving the man with joystick and stuff (idk how to do it)
 //making ghosts and moving them
 //detecting collison and losing game
 //can it be potentially done with i2c? 





