This is the first functional sketch for the project H.Y.F.Y.L.C.!

A state-machine sketch has been implemented for the project.
A MyTimer class is used to instantiate the objects that manage the timing of different elements such as motors, status LED and button de-bouncing.

Each states and a description of the activities they manage are listed below.

STATE 0: IDLE

    Waits for the buttons to be pressed.
        Button 1: Starts the main cycle.
        Buttons 1 and 2 pressed for more than 2 seconds: enters calibration state 4.

STATE 1: RUN

    Starts the food loading motors for the time defined by loadingTime.
    Reverses rotation at the end of each loading interval to avoid obstructions by calling state 2.
    Reads the button presses:
        Button 1: Stops loading and returns to state 0
        Button 2: Goes to state 2 to remove the obstructions.
    Reads the photo barrier and detects the food passage then stops the food loading motor then goes to state 3.
    A maximum numer of loading attempts with no food transit detection is set by the loadAttempts define.

STATE 3: LAUNCH

    Activates the motor that loads the bat spring for a time defined by springChargeTime.
    Waits 500 ms before returning to state 1.

STATE 4: CALIBRATION

    Reads the analog input connected to the photo barrier and activates the status LED in a fixed manner if the value read is close to the value defined by lightBaseline with a tolerance defined by baselineTolerance.
    Reads the button presses:
        Button 2: exits calibration and returns to state 0.

Egidio Fabris - 25 apr 2025
