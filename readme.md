# Tic Tac Toe

This is code for our machine which plays tic tac toe.


It is cheesey. But also grate. Hopefully the kids will love it.

# Gameplay

To begin, the human draws a red X on their board. They place the board into the machine, then press the "Play" button.

The robot analyzes the board state, chooses where to place an O, and draws it. The human takes the board and draws another X. 

The process repeats until one player has won the game.

# Algorithm

A medium-difficulty algorithm has been implemented:
 - If the robot (O's) can win with a single move, the robot will make that move
 - If the human (X's) can win with a single move, the robot will block that move
 - Otherwise, the robot will place in a random location.

# Structure


## JeVois Vision Camera

 - Takes images of the board to determine which spaces are populated by the robot or the human
 - Recieves command from the Arduino to process the present image
 - Transmits board state to the Arduino
 
 ## Arduino
 
  - Mega 
  - Maintains state machine for full robot state
  - On Play button press, sends command to JeVois to read board state
  - Listens for and Parses JeVois response
  - Calculates mark placement location
  - Plans stepper motor paths to draw the mark 
  - Sends commands to stepper motor driver to execute draw
  
  ## Physical Motor
   
   - 3 axis pen moving rig
   
  # Credits
  
  Some days you eat the cheese, some days the cheese eats you.
