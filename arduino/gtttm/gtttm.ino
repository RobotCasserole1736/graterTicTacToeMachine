#include "AccelStepper.h"

/*
 * The Grater Tic Tac Toe Machine
 * Plays tic tac toe and such.
 * It's Cheesy and Grate. mmmmmm
 * 
 *          _--"-.
      .-"      "-.
     |""--..      '-.
     |      ""--..   '-.
     |.-. .-".    ""--..".
     |'./  -_'  .-.      |
     |      .-. '.-'   .-'
     '--..  '.'    .-  -.
          ""--..   '_'   :
                ""--..   |
                      ""-' 
 * 
 * 
 * See License.txt in the root of the repo!
 */

////////////////////////////////////////////////////////////
// Debug print switches
////////////////////////////////////////////////////////////
//Uncomment this to get a bunch more spammed to the debug serial port
//#define DEBUG_POS


////////////////////////////////////////////////////////////
// Constant Values
////////////////////////////////////////////////////////////

//Physical hardware IO def's
#define X_STEPPER_PIN_1 3
#define X_STEPPER_PIN_2 4
#define X_STEPPER_PIN_3 5
#define X_STEPPER_PIN_4 6

#define Y_STEPPER_PIN_1 7
#define Y_STEPPER_PIN_2 8
#define Y_STEPPER_PIN_3 9
#define Y_STEPPER_PIN_4 10

#define Z_STEPPER_PIN_1 11
#define Z_STEPPER_PIN_2 12
#define Z_STEPPER_PIN_3 16
#define Z_STEPPER_PIN_4 14

//Stepper Operational mode defs
#define HALFSTEP 8

//Jevois interaface
#define JEVOIS_BOARD_STATE_REQ_STR "read\n"
#define JEVOIS_READ_TIMEOUT_MS 10000

//Other IO
#define PLAY_BUTTON_PIN 15
#define PLAY_BUTTON_DBNC_LOOPS 10 //Count to debounce the play button switch for
#define PLAY_BUTTON_PRESSED true //state that digitalRead should return when pressed
#define BUILT_IN_LED_PIN 13


//Motor Motion Constants.
// With the exception of steps/mm, all distances in mm
#define STEPS_PER_MM 48
#define GRID_X_OFFSET 10.0
#define GRID_Y_OFFSET 10.0

#define PEN_UP_POS 0.0
#define PEN_DOWN_POS 10.0
#define CELL_X_SIZE 10.0
#define CELL_Y_SIZE 10.0

#define DRAWN_CIRCLE_RADIUS CELL_X_SIZE/3.0
#define DRAWN_CIRCLE_APROX_STEPS 32 //Number of steps to use to approximate a circle with straight lines

#define MAX_SPEED_MM_PER_SEC 150
#define MAX_ACCEL_MM_PER_SEC2 150




//Operation states
enum opState_t{
  WAIT_FOR_USER,        //waiting for the user to hit the button to request a move
  REQ_BOARD_STATE,      //Request the board state from the Jevois
  WAIT_FOR_BOARD_STATE, //Wait for the Jevois to return the board state
  CALC_MOVE,            //Calculate which cell to put a mark in
  WAIT_FOR_MOVE         //Make the move,slick
};


////////////////////////////////////////////////////////////
// Global Vars
////////////////////////////////////////////////////////////

//The board is a 3x3 array of cells
// Each cell can be blank (_), have Robot mark (O), or a human mark (X)
// This will be populated by the JeVois's serial response.
//
// Board Numbering & Orientation Scheme:
//              |       |              Positive Z: Down into the paper
//          0   |   1   |   2          
//       _______|_______|_______       |
//              |       |             -+---> X
//          3   |   4   |   5          |
//       _______|_______|_______       |
//              |       |              V
//          6   |   7   |   8          y
//              |       |              
char BoardState[9];

//The present state of the arduino controller
opState_t State;

//Stepper Motor Objects
AccelStepper stepperX(HALFSTEP, X_STEPPER_PIN_1, X_STEPPER_PIN_2, X_STEPPER_PIN_3, X_STEPPER_PIN_4);
AccelStepper stepperY(HALFSTEP, Y_STEPPER_PIN_1, Y_STEPPER_PIN_2, Y_STEPPER_PIN_3, Y_STEPPER_PIN_4);
AccelStepper stepperZ(HALFSTEP, Z_STEPPER_PIN_1, Z_STEPPER_PIN_2, Z_STEPPER_PIN_3, Z_STEPPER_PIN_4);

//Play button debouncing
unsigned int playButtonDbncCounter = PLAY_BUTTON_DBNC_LOOPS;



////////////////////////////////////////////////////////////
// Movement Sequencer - Circle draw algorithm
////////////////////////////////////////////////////////////
#define CELL_UNDEFINED -1
int targetCell = CELL_UNDEFINED;

//In our sequencer, we move through a sequence of steps.
// 0 = move from home to first point of circle
// 1 = Lower pen
// 2 = move first point of circle to second point of circle
// ...
// DRAWN_CIRCLE_APROX_STEPS + 2 = move to final point on circle
// DRAWN_CIRCLE_APROX_STEPS + 3 = Raise pen
// DRAWN_CIRCLE_APROX_STEPS + 4 = move home
unsigned int sequenceStep = 0;

//home->circle, lower pen, draw circle, raise pen, circle->home
#define TOTAL_STEPS  (1 + 1 + DRAWN_CIRCLE_APROX_STEPS + 1 + 1)

//True if we're moving, false if we're done moving
bool moveInProcess = false;
bool newStep = false;

//"Origin" of the target cell - top-right corner
float tgtCellXPos = 0;
float tgtCellYPos = 0;

//Center of the target cell
float tgtCellXCenter = 0;
float tgtCellYCenter = 0;

//Circle points
float circleXCoords[DRAWN_CIRCLE_APROX_STEPS+1] = {0};
float circleYCoords[DRAWN_CIRCLE_APROX_STEPS+1] = {0};

//Sets a new target cell to draw an output in
//Calculates path constants while it's at it.
void mvSeqSetTargetCell(int tgt_in){
  targetCell = tgt_in;

  //Calc the origin of the target cell
  tgtCellXPos = GRID_X_OFFSET + (CELL_X_SIZE * (float)(tgt_in % 3));
  tgtCellYPos = GRID_Y_OFFSET + (CELL_Y_SIZE * (float)(tgt_in / 3));
  
  //calc the center of the cell
  tgtCellXCenter = tgtCellXPos + CELL_X_SIZE/2.0;
  tgtCellYCenter = tgtCellYPos + CELL_Y_SIZE/2.0;

  //Calc the points on the circle
  for(int pointIdx = 0; pointIdx < DRAWN_CIRCLE_APROX_STEPS+1;  pointIdx++){
    float angleRad = ((float)(pointIdx)) / ((float)DRAWN_CIRCLE_APROX_STEPS) * 2.0 * 3.14159;
    circleXCoords[pointIdx] = tgtCellXCenter + DRAWN_CIRCLE_RADIUS * sin( angleRad );
    circleYCoords[pointIdx] = tgtCellYCenter + DRAWN_CIRCLE_RADIUS * cos( angleRad );
  }

  moveInProcess = true;
  sequenceStep = 0;
  newStep = true;

  Serial.println("Finished calculating new path.");
}


unsigned int updateCtr = 0;

void mvSeqUpdate(){
  //Circle-draw algorithm
  if(newStep == true){
     Serial.print("Running step ");
     Serial.print(sequenceStep);
     Serial.println("...");

    if(sequenceStep == 0){
      //Move to start of circle
      stepperX.moveTo(mmToSteps(circleXCoords[0]));
      stepperY.moveTo(mmToSteps(circleYCoords[0]));
      stepperZ.moveTo(mmToSteps(PEN_UP_POS));
    } else if(sequenceStep == 1){
      //Lower pen
      stepperX.moveTo(mmToSteps(circleXCoords[0]));
      stepperY.moveTo(mmToSteps(circleYCoords[0]));
      stepperZ.moveTo(mmToSteps(PEN_DOWN_POS));
    } else if(sequenceStep >= 2 && sequenceStep <= DRAWN_CIRCLE_APROX_STEPS + 2){
      //Draw circle
      stepperX.moveTo(mmToSteps(circleXCoords[sequenceStep-2]));
      stepperY.moveTo(mmToSteps(circleYCoords[sequenceStep-2]));
      stepperZ.moveTo(mmToSteps(PEN_DOWN_POS));
    } else if(sequenceStep == DRAWN_CIRCLE_APROX_STEPS + 3){
      //Raise pen
      stepperX.moveTo(mmToSteps(circleXCoords[DRAWN_CIRCLE_APROX_STEPS]));
      stepperY.moveTo(mmToSteps(circleYCoords[DRAWN_CIRCLE_APROX_STEPS]));
      stepperZ.moveTo(mmToSteps(PEN_UP_POS));
    }else if(sequenceStep == DRAWN_CIRCLE_APROX_STEPS + 4){
      //Move home
      stepperX.moveTo(mmToSteps(0));
      stepperY.moveTo(mmToSteps(0));
      stepperZ.moveTo(mmToSteps(PEN_UP_POS));
    } else {
      //IDK let's go home
      stepperX.moveTo(mmToSteps(0));
      stepperY.moveTo(mmToSteps(0));
      stepperZ.moveTo(mmToSteps(PEN_UP_POS));
    }
  }

  //If we're at 0 distance-to-go, we can move on to the next step
  if(stepperX.isRunning() == false &&
     stepperY.isRunning() == false &&
     stepperZ.isRunning() == false ) {

       sequenceStep++;
       newStep = true;
       moveInProcess = true;

       Serial.print("Finished step ");
       Serial.println(sequenceStep-1);
       if( sequenceStep > TOTAL_STEPS){
         //All done, clean up variable state
         moveInProcess = false;
         newStep = false;
         sequenceStep = 0;
       }
       
  } else {
    newStep = false;
    moveInProcess = true;
    
  #ifdef DEBUG_POS
    Serial.println("\n\n===============");
    Serial.print("POS X = ");
    Serial.println(stepperX.currentPosition());
    Serial.print("POS Y = ");
    Serial.println(stepperY.currentPosition());
    Serial.print("POS Z = ");
    Serial.println(stepperZ.currentPosition());
  #endif
  }

}

void mvSeqOutputUpdate(){
  //Do periodic output updates
  stepperX.run();
  stepperY.run();
  stepperZ.run();
}



////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////
void resetState(){
  State = WAIT_FOR_USER;
  for(int cellIdx = 0; cellIdx < 9; cellIdx++){
    BoardState[cellIdx] = "_";
  }

  targetCell = CELL_UNDEFINED;
}

//Returns an index 0-8 to say where the robot should place a 
// mark. Should calculate this based on BoardState.
int moveCalculator(){
  //TODO: Fill me in with logic to win against the kids, but not too hard

  //Temp: Stupid robot.
  return 8;

}

//converts mm to steps
float mmToSteps(float mm){
  return STEPS_PER_MM * mm;
}

void printBoardState(void){
  Serial.println("Board State:");
  Serial.println("       |     |     ");   
  Serial.print("    ");Serial.print(BoardState[0]);Serial.print("  |  ");Serial.print(BoardState[1]);Serial.print("  |  ");Serial.print(BoardState[2]);Serial.print("  \n");
  Serial.println("       |     |     "); 
  Serial.println("  -----+-----+-----"); 
  Serial.println("       |     |     "); 
  Serial.print("    ");Serial.print(BoardState[3]);Serial.print("  |  ");Serial.print(BoardState[4]);Serial.print("  |  ");Serial.print(BoardState[5]);Serial.print("  \n");
  Serial.println("       |     |     "); 
  Serial.println("  -----+-----+-----"); 
  Serial.println("       |     |     "); 
  Serial.print("    ");Serial.print(BoardState[6]);Serial.print("  |  ");Serial.print(BoardState[7]);Serial.print("  |  ");Serial.print(BoardState[8]);Serial.print("  \n");
  Serial.println("       |     |     "); 
}




////////////////////////////////////////////////////////////
// Main Arduino Functions
////////////////////////////////////////////////////////////

void setup() {

  //Serial setup
  Serial.begin(115200); //Debug serial port
  Serial1.begin(115200); //JeVois Communication

  Serial.println("+++++++++++++++++++++++++++++++++++++++");
  Serial.println("+++  WELCOME TO GRATER TIC TAC TOE  +++");
  Serial.println("+++++++++++++++++++++++++++++++++++++++");

  //Init Globals
  resetState();

  //configure Pins
  pinMode(PLAY_BUTTON_PIN, INPUT);
  pinMode(BUILT_IN_LED_PIN, OUTPUT);

  //Configure Stepper motors
  stepperX.setMaxSpeed(mmToSteps(MAX_SPEED_MM_PER_SEC));
  stepperX.setAcceleration(mmToSteps(MAX_ACCEL_MM_PER_SEC2));

  stepperY.setMaxSpeed(mmToSteps(MAX_SPEED_MM_PER_SEC));
  stepperY.setAcceleration(mmToSteps(MAX_ACCEL_MM_PER_SEC2));

  stepperZ.setMaxSpeed(mmToSteps(MAX_SPEED_MM_PER_SEC));
  stepperZ.setAcceleration(mmToSteps(MAX_ACCEL_MM_PER_SEC2));

  Serial.println("Finished Init");
}

void loop() {
  bool boardStateRXed = false;
  
  //Main state machine
  opState_t prevState = State;

  //handle in-state logic
  switch(State){
    case WAIT_FOR_USER:
      //Debounce the play button
      if(digitalRead(PLAY_BUTTON_PIN) == PLAY_BUTTON_PRESSED){
        if(playButtonDbncCounter != 0){
          playButtonDbncCounter--;
        }
      } else {
        playButtonDbncCounter = PLAY_BUTTON_DBNC_LOOPS;
      }

      //Flush the jevois serial port
      while(Serial1.available() != 0){
        Serial1.read(); //Discard remaining buffer contents
      }

    break;

    case REQ_BOARD_STATE:
      //Send command to JeVois
      Serial1.write(JEVOIS_BOARD_STATE_REQ_STR);
    break;

    case WAIT_FOR_BOARD_STATE:
      if(Serial1.available() >= 10){
        for(int cellIdx = 0; cellIdx < 9; cellIdx++){
          BoardState[cellIdx] = Serial1.read();
          
        }
        boardStateRXed = true;
        while(Serial1.available() != 0){
          Serial1.read(); //Discard remaining buffer contents
        }

        //DEBUG ONLY EMOVE ME WHEN DONE
        BoardState[0] = ' ';
        BoardState[1] = 'X'; 
        BoardState[2] = 'O'; 
        BoardState[3] = 'O'; 
        BoardState[4] = 'O'; 
        BoardState[5] = ' '; 
        BoardState[6] = 'X'; 
        BoardState[7] = ' '; 
        BoardState[8] = 'X';  
      }
    break;

    case CALC_MOVE:
      mvSeqSetTargetCell(moveCalculator());
    break;

    case WAIT_FOR_MOVE:
      mvSeqUpdate();
    break;

    default:

    break;
  }



  //Calc new state
  switch(State){
    case WAIT_FOR_USER:
      if(playButtonDbncCounter == 0){
        Serial.println("Requesting board state from Jevois...");
        State = REQ_BOARD_STATE;
      } else {
        State = WAIT_FOR_USER;
      }
    break;

    case REQ_BOARD_STATE:
      State = WAIT_FOR_BOARD_STATE;
    break;

    case WAIT_FOR_BOARD_STATE:
      if(boardStateRXed){
        Serial.println("Got board state, calculating move...");
        printBoardState();
        State = CALC_MOVE;
      } else {
        State = WAIT_FOR_BOARD_STATE;
      }
    break;

    case CALC_MOVE:
      Serial.println("Performing move...");
      State = WAIT_FOR_MOVE;
    break;

    case WAIT_FOR_MOVE:
      if(moveInProcess == false){
        Serial.println("Move complete! Waiting for user input...");
        State = WAIT_FOR_USER;
      } else {
        State = WAIT_FOR_MOVE;
      }
    break;

    default:
      State = WAIT_FOR_USER; //if all else fails, go back to the init state
    break;
  }


  mvSeqOutputUpdate();

}
