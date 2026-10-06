// Smart Traffic Light
// Finite State Machine with Pedestrian Button
//
// This project controls two traffic lights:
//   1. North-South traffic
//   2. East-West traffic
//
// The lights change automatically based on time
// A pedestrian button can interrupt the normal cycle and turn on a WALK LED
//
// Main concepts used:
//   - Finite State Machine (FSM)
//   - enum
//   - millis() for non-blocking timing
//   - Hardware interrupt

// PIN DEFINITIONS
// North-South traffic light
const int NS_RED    = 4;
const int NS_YELLOW = 5;
const int NS_GREEN  = 6;
// East-West traffic light
const int EW_RED    = 7;
const int EW_YELLOW = 8;
const int EW_GREEN  = 9;
// Pedestrian system
const int WALK_LED = 10;
const int PED_BTN  = 2;     // Pin 2 supports hardware interrupts



// TRAFFIC LIGHT STATES
// The traffic light can only be in one state at a time
// enum lets me use names such as NS_GREEN_STATE instead of using numbers such as 0, 1, 2, 3 and 4

enum TrafficState {
  NS_GREEN_STATE,       // State 1
  NS_YELLOW_STATE,      // State 2
  EW_GREEN_STATE,       // State 3
  EW_YELLOW_STATE,      // State 4
  PEDESTRIAN_STATE      // State 5
};


// Start the system with North-South green
TrafficState currentState = NS_GREEN_STATE;

// TIMING
// millis() is used instead of delay()
// delay() stops the normal program from continuing while it waits. A hardware interrupt can still occur, but the main FSM cannot process the request until the delay ends
// millis() lets loop() keep running while time is measured

const unsigned long GREEN_DURATION  = 5000;   // 5 seconds
const unsigned long YELLOW_DURATION = 2000;   // 2 seconds
const unsigned long PED_DURATION    = 4000;   // 4 seconds

// Stores the time when the current state started
unsigned long stateStartTime = 0;


// PEDESTRIAN REQUEST
// This variable becomes true when the button is pressed
// volatile is used because the variable can be changed by the interrupt outside the normal program flow

volatile bool pedRequest = false;

// INTERRUPT SERVICE ROUTINE (ISR)
// This function runs when the pedestrian button is pressed
// The ISR is kept very short
// It only records that a pedestrian request happened
// The main program handles the actual state change

void pedestrianRequest() {
  pedRequest = true;
}



// TURN ALL LEDS OFF
// Before changing to a new state, all LEDs are turned off
// Then only the LEDs needed for the new state are turned on

void allOff() {

  digitalWrite(NS_RED, LOW);
  digitalWrite(NS_YELLOW, LOW);
  digitalWrite(NS_GREEN, LOW);

  digitalWrite(EW_RED, LOW);
  digitalWrite(EW_YELLOW, LOW);
  digitalWrite(EW_GREEN, LOW);

  digitalWrite(WALK_LED, LOW);
}


// APPLY THE CURRENT STATE
// This function decides which LEDs should be ON for each traffic-light state

void applyState(TrafficState state) {

  // Start by turning everything off
  allOff();

  switch (state) {

    // STATE 1 - NORTH-SOUTH GREEN

    case NS_GREEN_STATE:

      // North-South traffic can move
      // East-West traffic must stop

      digitalWrite(NS_GREEN, HIGH);
      digitalWrite(EW_RED, HIGH);

      break;


    // STATE 2 - NORTH-SOUTH YELLOW

    case NS_YELLOW_STATE:

      // North-South traffic is preparing to stop
      // East-West traffic is still stopped

      digitalWrite(NS_YELLOW, HIGH);
      digitalWrite(EW_RED, HIGH);

      break;


    // STATE 3 - EAST-WEST GREEN

    case EW_GREEN_STATE:

      // East-West traffic can move
      // North-South traffic must stop

      digitalWrite(EW_GREEN, HIGH);
      digitalWrite(NS_RED, HIGH);

      break;


    // STATE 4 - EAST-WEST YELLOW

    case EW_YELLOW_STATE:

      // East-West traffic is preparing to stop
      // North-South traffic is still stopped

      digitalWrite(EW_YELLOW, HIGH);
      digitalWrite(NS_RED, HIGH);

      break;


    // STATE 5 - PEDESTRIAN

    case PEDESTRIAN_STATE:

      // Both traffic directions are stopped
      // The pedestrian WALK light is turned on

      digitalWrite(NS_RED, HIGH);
      digitalWrite(EW_RED, HIGH);
      digitalWrite(WALK_LED, HIGH);

      break;
  }
}


// DECIDE THE NEXT STATE

// This function checks:
//   1. Was the pedestrian button pressed?
//   2. Has enough time passed for the current state?

// It returns the state that should come next

TrafficState nextState(TrafficState state, unsigned long elapsed) {

  // If already in pedestrian mode

  if (state == PEDESTRIAN_STATE) {

    // Ignore extra button presses while pedestrians are already crossing
    pedRequest = false;

    // After 4 seconds, return to North-South green
    if (elapsed >= PED_DURATION) {

      return NS_GREEN_STATE;
    }

    // Otherwise stay in pedestrian mode
    return PEDESTRIAN_STATE;
  }


  // Check for a pedestrian request

  if (pedRequest) {

    // Clear the request after it has been accepted
    pedRequest = false;

    // Move to the pedestrian state
    return PEDESTRIAN_STATE;
  }

  // Normal timed traffic-light transitions

  switch (state) {


    case NS_GREEN_STATE:

      // After 5 seconds:
      // NS Green -> NS Yellow

      if (elapsed >= GREEN_DURATION) {

        return NS_YELLOW_STATE;
      }

      break;


    case NS_YELLOW_STATE:

      // After 2 seconds:
      // NS Yellow -> EW Green

      if (elapsed >= YELLOW_DURATION) {

        return EW_GREEN_STATE;
      }

      break;


    case EW_GREEN_STATE:

      // After 5 seconds:
      // EW Green -> EW Yellow

      if (elapsed >= GREEN_DURATION) {

        return EW_YELLOW_STATE;
      }

      break;


    case EW_YELLOW_STATE:

      // After 2 seconds:
      // EW Yellow -> NS Green

      if (elapsed >= YELLOW_DURATION) {

        return NS_GREEN_STATE;
      }

      break;


    case PEDESTRIAN_STATE:

      // This state was already handled above
      break;
  }


  // If no transition is needed, stay in the same state.
  return state;
}

// SETUP

// setup() runs once when the Arduino starts

void setup() {


  // Set all traffic LEDs as outputs.

  pinMode(NS_RED, OUTPUT);
  pinMode(NS_YELLOW, OUTPUT);
  pinMode(NS_GREEN, OUTPUT);

  pinMode(EW_RED, OUTPUT);
  pinMode(EW_YELLOW, OUTPUT);
  pinMode(EW_GREEN, OUTPUT);

  pinMode(WALK_LED, OUTPUT);


  // PEDESTRIAN BUTTON

  // INPUT_PULLUP uses Arduino's internal pull-up resistor
  // Button not pressed = HIGH
  // Button pressed     = LOW

  pinMode(PED_BTN, INPUT_PULLUP);


  // HARDWARE INTERRUPT

  // Pin 2 is connected to the pedestrian button

  // FALLING means the interrupt happens when the signal changes from HIGH to LOW, which happens when the button is pressed

  attachInterrupt(
    digitalPinToInterrupt(PED_BTN),
    pedestrianRequest,
    FALLING
  );


  // Start the Serial Monitor
  Serial.begin(9600);


  // Start the FSM in State 1
  currentState = NS_GREEN_STATE;

  // Record when State 1 started
  stateStartTime = millis();

  // Turn on the correct LEDs for State 1
  applyState(currentState);


  Serial.println("Smart Traffic Light Started");
  Serial.println("State: NS_GREEN");
}


// MAIN LOOP

// loop() runs continuously.

// It checks the time and determines whether the traffic light needs to move to another state

void loop() {


  // STEP 1 - Calculate elapsed time

  // millis() gives the total time since Arduino started
  // Subtracting stateStartTime tells us how long the current traffic-light state has been active

  unsigned long elapsed =
      millis() - stateStartTime;


  // STEP 2 - Find out what state should come next

  TrafficState next =
      nextState(currentState, elapsed);


  // STEP 3 - Change state if needed

  if (next != currentState) {


    // Update the current state
    currentState = next;


    // Start timing the new state from zero
    stateStartTime = millis();


    // Change the LEDs to match the new state
    applyState(currentState);


    // Print the new state to the Serial Monitor

    Serial.print("State: ");

    switch (currentState) {

      case NS_GREEN_STATE:

        Serial.println("NS_GREEN");
        break;


      case NS_YELLOW_STATE:

        Serial.println("NS_YELLOW");
        break;


      case EW_GREEN_STATE:

        Serial.println("EW_GREEN");
        break;


      case EW_YELLOW_STATE:

        Serial.println("EW_YELLOW");
        break;


      case PEDESTRIAN_STATE:

        Serial.println("PEDESTRIAN");
        break;
    }
  }

  // There is no delay() here
  // loop() immediately starts again and continues checking the traffic light and pedestrian request
}