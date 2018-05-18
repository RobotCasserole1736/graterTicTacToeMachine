#include "AccelStepper.h"

/*
 * The Grater Tic Tac Toe Machine
 * Plays tic tac toe and such.
 * It's Cheesy and Grate.
 */

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
#define BUILT_IN_LED_PIN 13

//Motor Motion Constants.
// With the exception of steps/mm, all distances in mm
#define STEPS_PER_MM 10000
#define GRID_X_OFFSET 10.0
#define GRID_Y_OFFSET 10.0

#define PEN_UP_POS 0.0
#define PEN_DOWN_POS 10.0
#define CELL_X_SIZE 10.0
#define CELL_Y_SIZE 10.0

#define MAX_SPEED_MM_PER_SEC 200
#define MAX_ACCE_MM_PER_SEC2 20


//Derived position constants



//Operation states
enum opState_t{
  WAIT_FOR_USER,        //waiting for the user to hit the button to request a move
  REQ_BOARD_STATE,      //Request the board state from the Jevois
  WAIT_FOR_BOARD_STATE, //Wait for the Jevois to return the board state
  CALC_MOVE,            //Calculate which cell to put a mark in
  WAIT_FOR_MOVE         //Make the move,slick
}


////////////////////////////////////////////////////////////
// Global Vars
////////////////////////////////////////////////////////////

//The board is a 3x3 array of cells
// Each cell can be blank (_), have Robot mark (O), or a human mark (X)
// This will be populated by the JeVois's serial response.
char BoardState[3][3];

//The present state of the arduino controller
opState_t State;

//Stepper Motor Objects
AccelStepper stepperX(HALFSTEP, X_STEPPER_PIN_1, X_STEPPER_PIN_2, X_STEPPER_PIN_3, X_STEPPER_PIN_4);
AccelStepper stepperY(HALFSTEP, Y_STEPPER_PIN_1, Y_STEPPER_PIN_2, Y_STEPPER_PIN_3, Y_STEPPER_PIN_4);
AccelStepper stepperZ(HALFSTEP, Z_STEPPER_PIN_1, Z_STEPPER_PIN_2, Z_STEPPER_PIN_3, Z_STEPPER_PIN_4);

////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////
void resetState(){
  State = WAIT_FOR_USER;
  BoardState[0][0] = '_';
  BoardState[0][1] = '_';
  BoardState[0][2] = '_';
  BoardState[1][0] = '_';
  BoardState[1][1] = '_';
  BoardState[1][2] = '_';
  BoardState[2][0] = '_';
  BoardState[2][1] = '_';
  BoardState[2][2] = '_';
}

//converts mm to steps
float mmToSteps(float mm){
  return STEPS_PER_MM * mm;
}




////////////////////////////////////////////////////////////
// Main Arduino Functions
////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200); //Debug serial port
  Serial1.begin(115200); //JeVois Communication

  //Init Globals
  resetState();

  //Init IO
  stepper1.setMaxSpeed(mmToSteps(MAX_SPEED_MM_PER_SEC));
  stepper1.setAcceleration(mmToSteps(MAX_ACCEL_MM_PER_SEC2));
}

void loop() {
  


  //Do periodic output updates
  stepperX.run();
  stepperY.run();
  stepperZ.run();
}
