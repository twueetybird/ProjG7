
#include <TimerOne.h>         // timer interrupt
#include <Adafruit_GFX.h>     // drawing shapes
#include <MCUFRIEND_kbv.h>    // TFT 
#include <LiquidCrystal.h>    // second LCD

//TFT+LCD
MCUFRIEND_kbv tft;
LiquidCrystal lcd(25, 27, 35, 33, 31, 29);

//colors + cell size
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define CELL_SIZE 40

//game starting 
int pacManX = 0, pacManY = 0;               
int ghostX = CELL_SIZE * 6, ghostY = CELL_SIZE * 4;    
int ghostTwoX =CELL_SIZE * 5 , ghostTwoY=CELL_SIZE * 4;                  
int score = 0;                             
int level = 1;                            
bool gameStarted = false;   

//input pins
int xPin = A8;                             // Joystick 
int yPin = A9;                             // Joystick 
int buttonPin = 23;                        // Joystick button pin
bool buttonPressed = false;                // Tracks button state
bool lastButtonState = HIGH;               // Last button state
unsigned long lastDebounceTime = 0;        // Debounce timer
const unsigned long debounceDelay = 50;    // Debounce delay

//moving timers
unsigned int lastMoveTime = 0;             // Last time pac moved
const int moveInterval = 200;              // how long before readin next joystick input 
unsigned int lastMoveTimeGhost = 0;        // Last time ghose moved
const int ghostMoveInterval = 500;         // Ghost movement interval 

const int SPEAKER = 29;

//maze
#define ROWS 8
#define COLS 12
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



void setup() {
    
    Serial.begin(9600);
    tft.begin(tft.readID());
    tft.setRotation(1);  // Landscape mode
    tft.fillScreen(BLACK);

    //lcd initialization
    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Your Score");

    pinMode(xPin, INPUT);
    pinMode(yPin, INPUT);
    pinMode(buttonPin, INPUT_PULLUP); //when button is not pressed the value read is high 
    pinMode(SPEAKER, OUTPUT);

    //interrupts:
    attachInterrupt(digitalPinToInterrupt(buttonPin), startGame, FALLING);
    Timer1.initialize(1000); // checks every sec
    Timer1.attachInterrupt(wonGame);

    //display initialization:
    tft.setCursor(50, 100);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Press the button");
    tft.setCursor(50, 120);
    tft.print("to start the game.");


}

void loop() {
  
  //debounce button detection: 
  handleButtonDebounce();

  if (gameStarted) {
    xVal = analogRead(xPin);
    yVal = analogRead(yPin);
    
    //debugging the joyStick:
    Serial.print("X: ");
    Serial.print(xVal);
    Serial.print(" | Y: ");
    Serial.println(yVal);


    movePacWJoy();
    trackingScores();
    moveGhost();
    checkGhostCollision();

    // Update second lcd
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(score);
  }

}

//this used to be in the loop but I moved here, cause it makes the code easier to read
void handleButtonDebounce() {
    
  int currentButtonState = digitalRead(buttonPin);

  if (currentButtonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (currentButtonState == LOW && !buttonPressed) {
        startGame();
        buttonPressed = true;
    } else if (currentButtonState == HIGH) {
        buttonPressed = false;
    }
  }
    lastButtonState = currentButtonState;
}

void startGame() {

  gameStarted = true;
  score = 0;
  pacManX = pacManY = 0;

  if(level==1){

    ghostX = CELL_SIZE * 6;
    ghostY = CELL_SIZE * 4;

  } else if (level==2){

    ghostX = CELL_SIZE * 6;
    ghostY = CELL_SIZE * 4;

    ghostTwoX = CELL_SIZE * 5;
    ghostTwoX = CELL_SIZE * 4;

  }

  tone(SPEAKER, 440, 200); 
  delay(250);              
  tone(SPEAKER, 660, 200); 
  delay(250);              
  tone(SPEAKER, 880, 300); 


  // Draw the initial maze and characters
  drawMaze();
  drawPacman();   
  Serial.println("Game Started!");
}

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

void drawPacman(){
  tft.fillCircle(pacManX + CELL_SIZE / 2 , pacManY + CELL_SIZE / 2 , 10 , YELLOW);
}

void clearOut(){
  tft.fillCircle(pacManX + CELL_SIZE / 2, pacManY + CELL_SIZE / 2, 10, BLACK);  // Clearing the small circle
}

void drawGhost(){
  tft.fillCircle(ghostX + CELL_SIZE / 2 , ghostY + CELL_SIZE / 2 , CELL_SIZE/3 , MAGENTA); 

  if (level == 2) {
        tft.fillCircle(ghostTwoX + CELL_SIZE / 2, ghostTwoY + CELL_SIZE / 2, CELL_SIZE / 3, MAGENTA);
  }
}

void clearGhost(){
  tft.fillCircle(ghostX + CELL_SIZE / 2 , ghostY + CELL_SIZE / 2 , CELL_SIZE/3 , BLACK);

  if (level == 2) {
        tft.fillCircle(ghostTwoX + CELL_SIZE / 2, ghostTwoY + CELL_SIZE / 2, CELL_SIZE / 3,BLACK);
  }
}

void moveGhost() {
  
  unsigned int currentTimeGhost = millis();

  if(currentTimeGhost - lastMoveTimeGhost > ghostMoveInterval) {

    clearGhost(); //clearing the maze so we dont end up having two ghosts

    int newGhostX = ghostX;
    int newGhostY = ghostY;

    int direction = random(4); //0=up 1= down 2= right 3=left /(-1) is there because we count from 0 / it should not be 1, 1 is wall in the maze!

    if(direction == 0) {
        newGhostY -= CELL_SIZE;

    } else if(direction == 1) {
        ewGhostY += CELL_SIZE;

    } else if(direction == 2) {
        newGhostX += CELL_SIZE;

    } else if(direction == 3) {
        newGhostX -= CELL_SIZE;

    }
    
    int newRow = newGhostY / CELL_SIZE;
    int newCol = newGhostX / CELL_SIZE;

    if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS) {
        if (maze[newRow][newCol] != 1) {  // 1 is the wall
            // Update ghost position if not a wall
            ghostX = newGhostX;
            ghostY = newGhostY;
        }
    }

    //if we are in the second level then we add ghost two 
    if(level == 2) {
        int newGhostTwoX = ghostTwoX;
        int newGhostTwoY = ghostTwoY;
        int direction = random(4); //0=up 1= down 2= right 3=left /(-1) is there because we count from 0 / it should not be 1, 1 is wall in the maze!

        if(direction == 0) {
            newGhostTwoY -= CELL_SIZE;

        } else if(direction == 1) {
            newGhostTwoY += CELL_SIZE;

        } else if(direction == 2) {
            newGhostTwoX += CELL_SIZE;

        } else if(direction == 3) {
            newGhostTwoX -= CELL_SIZE;

        }
    
    int newRowTwo = newGhostTwoY / CELL_SIZE;
    int newColTwo = newGhostTwoX / CELL_SIZE;
    if (newRowTwo >= 0 && newRowTwo < ROWS && newColTwo >= 0 && newColTwo < COLS) {
        if (maze[newRowTwo][newColTwo] != 1) {  // 1 is the wall
            // Update ghost position if not a wall
            ghostTwoX = newGhostTwoX;
            ghostTwoY = newGhostTwoY;
        }
    }
   }
  }
  drawGhost();
  lastMoveTimeGhost = currentTimeGhost;
}


void checkGhostCollision() {
  if ((pacManX == ghostX && pacManY == ghostY) ||( level == 2 && pacManX == ghostTwoX && pacManY == ghostTwoY)) {
    tone(SPEAKER, 220, 500); // A3 for 500ms (low pitch)
    delay(500);              // Wait
    tone(SPEAKER, 110, 1000); // A2 for 1 second (even lower pitch)

    tft.fillScreen(BLACK);
    tft.setCursor(180, 100);
    tft.setTextColor(RED);
    tft.setTextSize(2);
    tft.print("GAME OVER!");
    gameStarted = false;
  }
}

void trackingScores(){
 int row = pacManX/CELL_SIZE;
 int col = pacManY/CELL_SIZE;

 if(maze[row][col]==2){
    maze[row][col]=0; // updating the maze  
    tft.fillRect(row*CELL_SIZE, col*CELL_SIZE, CELL_SIZE, CELL_SIZE, BLACK);//updating the visuals
    score++;
 }
}

void movePacWJoy() {
  
  unsigned int currentTime = millis(); // Measures the current time

  if (currentTime - lastMoveTime > moveInterval) {
    
    xVal = analogRead(xPin);
    yVal = analogRead(yPin);
    
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

    // draw Pac-Man at new position
     drawPacman();

    // Track score if Pac-Man collects a pellet
    trackingScores();

    // Update the last movement time
      lastMoveTime = currentTime;
    }
}


bool wonGame() {

 for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (maze[i][j] == 2) {
                return false; 
            }
        }
  }
    
    //if you havent leveled up yet, you level up, else the final message pops off
  if(level==1){
    level = 2; 
    startGame();
  } else {

    tft.fillScreen(BLACK);
    tft.setCursor(50, 150);
    tft.setTextColor(GREEN);
    tft.setTextSize(2);
    tft.print("You Win!");
    gameStarted = false; // End game on win
  }
    return true; 
}








