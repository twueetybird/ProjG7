#include <Adafruit_GFX.h> //library used for drawing shapes 
#include <MCUFRIEND_kbv.h> //library used for the screen 
#include <TouchScreen.h> //library used for touchscreen
#include <LiquidCrystal.h> //this is for the second display we used 
MCUFRIEND_kbv tft;

LiquidCrystal lcd(25, 27, 35, 33, 31, 29);

int time = 0;//this is for testing the screen 



// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP = 6, XM = A2, YP = A1, YM = 7; //ID=0x9341

//touchscreen
TouchScreen ts = TouchScreen(6, A1, A2, 7, 300);
#define MINPRESSURE 200
#define MAXPRESSURE 1000
const int TS_LEFT = 907, TS_RT = 136, TS_TOP = 942, TS_BOT = 139;

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

bool buttonPressed = false;  // Flag to track if button is pressed
bool lastButtonState = HIGH;  // Last button state (using INPUT_PULLUP, HIGH means button not pressed)
unsigned long lastDebounceTime = 0;  // Last time button state changed
unsigned long debounceDelay = 50;  // debounce delay time (in milliseconds)
//interrupt 
volatile bool gameStarted = false;  // indicate if the game has started

#define CELL_SIZE 40

int pacManX = 0, pacManY = 0; // Pac-Man's position
int ghostX = CELL_SIZE * 6, ghostY = CELL_SIZE * 4; // Ghost's initial position
int score = 0; // Track score

//speaker
const int SPEAKER= 29;

void setup(void){

  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print("Time:");

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
  tft.fillScreen(BLACK);
  tft.setCursor(50, 100); // Set cursor for the first line
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.print("Press the button");

  tft.setCursor(50, 120); // Adjust Y-coordinate for the second line
  tft.print("to start the game.");


}

void loop() {

 // Debounced button press detection logic
  int currentButtonState = digitalRead(buttonPin);

  // Check if the button state has changed (HIGH to LOW or LOW to HIGH)
  if (currentButtonState != lastButtonState) {
    // Reset debounce timer
    lastDebounceTime = millis();
  }

  // If enough time has passed since the last state change, proceed
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button was pressed (button is LOW) and the game hasn't started yet
    if (currentButtonState == LOW && !buttonPressed) {
      Serial.println("Button Pressed");
      startGame();  // Call the function to start the game
      buttonPressed = true; // Set flag to prevent repeated prints
    } 
    // If the button is released (button is HIGH), reset flag to detect the next press
    else if (currentButtonState == HIGH) {
      buttonPressed = false;
    }

  }

lastButtonState = currentButtonState;


  


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

  lcd.setCursor(0,1);
  lcd.print(time);
  
  time++;
  

  } else {

    //tells the player to press the button so it starts 
    tft.setCursor(50, 150);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Please press the button in order");

    tft.setCursor(50, 180);
    tft.print("to start/restart the game.");
  }

  //this is for the second screen 
  lcd.setCursor(0,1);
  lcd.print(time);
  //delay (1000);
  time++;

 


}

//the function that gets called inside of the interrupt (by using this function we dont need to check if the button is pressed everytime in the main loop, it gets checked everytime here inside the loop)
void startGame() {
    if (!gameStarted) { // Check if the game has not already started
        gameStarted = true; // Set the gameStarted flag
        score = 0; // Reset score
        
        // Set fixed starting position for Pac-Man (ensure it's a valid empty space)
        pacManX = 1 * CELL_SIZE;  // Column 1 (second column)
        pacManY = 1 * CELL_SIZE;  // Row 1 (second row)

        // Check that the starting position is not a wall (value 1 in the maze)
        int row = pacManY / CELL_SIZE;
        int col = pacManX / CELL_SIZE;

        ghostX = CELL_SIZE * 6; // Reset ghost's position
        ghostY = CELL_SIZE * 4;

        // Clear the screen to remove any text or graphics
        tft.fillScreen(BLACK);

        // Draw the initial maze and characters
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


//Drawing the man obviously 
void drawPacman(){
  tft.fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , 10 , YELLOW);
}

//clearing the place where the man has been visually 
void clearOut(){
  tft.fillCircle(pacManX + CELL_SIZE / 2, pacManY + CELL_SIZE / 2, 10, BLACK);  // Clearing the small circle
}


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
    tft.fillScreen(BLACK);
    tft.setCursor(180, 100);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.print("GAME OVER!");
    gameStarted = false;
  }
}



//tracking score
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

void movePacWJoy() {
    unsigned int currentTime = millis(); // Measures the current time

    if (currentTime - lastMoveTime > moveInterval) {
        // Get joystick values
        xVal = analogRead(xPin);
        yVal = analogRead(yPin);

        // Debugging joystick input values
        Serial.print("X: ");
        Serial.print(xVal);
        Serial.print(" | Y: ");
        Serial.println(yVal);

        // Clear the previous position of Pac-Man
        clearOut();

        int newPacManX = pacManX;
        int newPacManY = pacManY;

        // Determine the new position based on joystick input
        if (xVal < 400) {
            newPacManX -= CELL_SIZE;  // Move left
        } else if (xVal > 600) {
            newPacManX += CELL_SIZE;  // Move right
        }

        if (yVal < 400) {
            newPacManY -= CELL_SIZE;  // Move up
        } else if (yVal > 600) {
            newPacManY += CELL_SIZE;  // Move down
        }

        // Convert Pac-Man's new coordinates to row/column in the maze
        int newRow = newPacManY / CELL_SIZE;
        int newCol = newPacManX / CELL_SIZE;

        // Check if the new position is within the bounds of the maze and not a wall
        if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS) {
            if (maze[newRow][newCol] != 1) {  // 1 is the wall
                // Update Pac-Man's position if not a wall
                pacManX = newPacManX;
                pacManY = newPacManY;
            }
        }

        // Redraw Pac-Man at the new position
        drawPacman();

        // Track score if Pac-Man collects a pellet
        trackingScores();

        // Update the last movement time
        lastMoveTime = currentTime;
    }
}








 


 //things we need to do :
 //for the joystick add interupt so that the game only starts when the buttonstate is on 
 //making ghosts and moving them
 //detecting collison and losing game
 //can it be potentially done with i2c? 





