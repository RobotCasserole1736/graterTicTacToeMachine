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
#define STEPS_PER_MM 500
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
AccelStepper stepperX(AccelStepper::FULL4WIRE, X_STEPPER_PIN_1, X_STEPPER_PIN_3, X_STEPPER_PIN_2, X_STEPPER_PIN_4);

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
  stepperX.setMaxSpeed(500);
  stepperX.move(1);

  stepperX.setAcceleration(400);
  stepperX.moveTo(20000);

  Serial.println("Finished Init");
}

void loop() {
  stepperX.run();

}
