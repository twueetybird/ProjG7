#include <Adafruit_GFX.h> //library used for drawing shapes 
#include <MCUFRIEND_kbv.h> //library used for the screen 
MCUFRIEND_kbv tft;

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341


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

//interrupt 
volatile bool gameStarted = false;  // indicate if the game has started


void setup(void){

//joystick 

pinMode(xPin, INPUT);
pinMode(yPin, INPUT);
pinMode(buttonPin, INPUT_PULLUP);

  Serial.begin(9600);
  uint16_t ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(1); // LANDSCAPE
  tft.fillScreen(BLACK);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(SPEAKER, OUTPUT);

  // the button is connected to a pullup resistor, meaning that, when not pressed the value is high, this means, that when you press it the value will be low (hence FALLING)

  attachInterrupt(digitalPinToInterrupt(buttonPin), startGame, FALLING);
  drawMaze();
  tft.setCursor(50, 100);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Press the button to start the game.");

}

void loop(void){

  


  if(gameStarted){

  //joystick
  xVal= analogRead(xPin);
  yVal = analogRead(yPin);
  

  //testing joystick 
  Serial.print("X: ");
  Serial.print(xVal);
  Serial.print(" | Y: ");
  Serial.print(yVal);
  Serial.print(" | Button: ");
  Serial.print(buttonState);
  delay(100);


  movePacWJoy();
  trackingScores(); 
  moveGhost();
  checkGhostCollision();
  

  } else {

    //tells the player to press the button so it starts 
    tft.setCursor(50, 100);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Please press the button in order to start/restart the game.");
  }

 


}

//the function that gets called inside of the interrupt (by using this function we dont need to check if the button is pressed everytime in the main loop, it gets checked everytime here inside the loop)
void startGame() {
  if (!gameStarted) {
    gameStarted = true;
    score = 0;
    pacManX = pacManY = 0; //reset pac
    ghostX = CELL_SIZE * 6; // Reset ghost position
    ghostY = CELL_SIZE * 4;
    tft.fillScreen(BLACK);
    drawMaze();
    drawPacman();
    Serial.println("Game Started!");
    
  }
}


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
                tft.fillRect(x, y, CELL_SIZE, CELL_SIZE, BLUE);
            } else if (maze[row][col] == 2) {
                // Draw a pellet (white circle)
                tft.fillCircle(x + CELL_SIZE / 2, y + CELL_SIZE / 2, 5, WHITE); //since it should be placed in the center it is cellsize/2 , the 5 shows radius, we can change that based on how big it should be. 
            } else {
                // Draw an empty space (black rectangle)
               tft.fillRect(x, y, CELL_SIZE, CELL_SIZE, BLACK);
            }
        }
    }
}

//drawing the pacman 
int pacManX = 0;
int pacManY= 0 ; 

//Drawing the man obviously 
void drawPacman(){
  tft.fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , 10 , YELLOW);
}

//clearing the place where the man has been visually 
void clearOut(){
 tft.fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , CELL_SIZE , BLACK);

}

//ghost (if we want to make more ghosts, we should add more to this)
int ghostX =  CELL_SIZE * 6; //col 6
int ghostY = CELL_SIZE * 4; //row 4

//drawing the ghost
void drawGhost(){
  tft.fillCircle(ghostX + CELL_SIZE / 2 , ghostY + CELL_SIZE / 2 , CELL_SIZE/3 , MAGENTA); //The ghost is a circle as well:(
}

//clearing out the ghost
void clearGhost(){
  tft.fillCircle(ghostX + CELL_SIZE / 2 , ghostY + CELL_SIZE / 2 , CELL_SIZE/3 , BLACK);
}

//now we want to move the ghost in 4 random directions, in given intervals (the random function generates a random number between 0 to 3 each time and then we assign a specific movement to each number ) 

//how often should the ghost move

unsigned int lastMoveTimeGhost=0;
const int ghostMoveInterval= 500; //we can always change this but I think it makes sense

void moveGhost(){

  unsigned int currentTimeGhost = millis();

  if(currentTimeGhost-lastMoveTimeGhost > ghostMoveInterval ){
  
  clearGhost(); //clearing the maze so we dont end up having two ghosts

  int direction = random(4); //0=up 1= down 2= right 3=left /(-1) is there because we count from 0 / it should not be 1, 1 is wall in the maze!

   int ghostCol = ghostX / CELL_SIZE; //ghosts col 
   int ghostRow = ghostY / CELL_SIZE; //ghosts row

  if((direction==0) && (ghostRow > 0) && (maze[ghostX / CELL_SIZE][(ghostY / CELL_SIZE) - 1] != 1)){
    ghostY -= CELL_SIZE;
  }
  if((direction==1) && (ghostRow < ROWS - 1) && (maze[ghostX / CELL_SIZE][(ghostY / CELL_SIZE) + 1] != 1)){
    ghostY+= CELL_SIZE;
  }
  if((direction==2) && (ghostCol < COLS - 1) && (maze[ghostX / CELL_SIZE +1][(ghostY / CELL_SIZE) ] != 1)){
    ghostX+= CELL_SIZE;
  }
    if((direction==3) && (ghostCol > 0) && (maze[ghostX / CELL_SIZE -1][(ghostY / CELL_SIZE) ] != 1)){
    ghostX-= CELL_SIZE;
  }

  drawGhost();
  lastMoveTimeGhost = currentTimeGhost;


  }
}

//deteccting collision

void checkGhostCollision() {
  if (pacManX == ghostX && pacManY == ghostY) {
    // Play lose sound
    tft.setCursor(50, 150);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.print("Game Over!");
    gameStarted = false;
  }
}



//tracking score
 int score = 0 ;
 void trackingScores(){
 int row = pacManX/CELL_SIZE;
 int col = pacManY/CELL_SIZE;

  if(maze[row][col]==2){
    maze[row][col]=0; // updating the maze  
    tft.fillRect(row*CELL_SIZE, col*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);//updating the visuals
    score++;
  }
 }
 
unsigned int lastMoveTime = 0; //shows the last time pac moved (it is unsigned, ang gets updated to current time as we go)
const int moveInterval = 200; //it has 200ms reaction time (so if you move it too fast it wont reat) , we can change this time (the lower it gets the faster it reacts)

void movePacWJoy(){

unsigned int currentTime = millis(); //measures the current move time, by calling the function millis

//if it is within the reactiontime
 if(currentTime-lastMoveTime>moveInterval){

  //get rid of the pac visually  and get ready for moving it further

  clearOut();

  if(xVal<400){

    pacManX-=CELL_SIZE; //move left, if in doubt look at the pic i uploaded in the doc

  } else if (xVal>600){

    pacManX+=CELL_SIZE;

  }

  if(yVal<400){

    pacManY-=CELL_SIZE;

  } else if (yVal>600){

    pacManY+=CELL_SIZE;
  }
 }


  //boundry checks so that it doesnt move out of the rows and cols: 
  //pacmans position (x or y), should be a number between 0 and (cols/rows * pixels in each cell), (-1 is there  beacause we count from 0 ofc:))
  constrain(pacManX, 0 , (COLS-1)*CELL_SIZE);
  constrain(pacManY, 0 , (ROWS-1)*CELL_SIZE);

  //draw him in the new pos
  trackingScores();
  drawPacman();

  //the last movement time, should be updated to the new movement time
  lastMoveTime=currentTime; 


 }

 


 //things we need to do :
 

 






