# Project 5: Traffic Light Controller

Arduino Uno traffic-light demonstration using a finite state machine, non-blocking `millis()` timing, and an interrupt-driven pedestrian button. Two LED groups represent North–South (NS) and East–West (EW) traffic. A white LED represents WALK.

## Operation

| State | NS light ON | EW light ON | WALK | Duration |
|---|---|---|---|---|
| NS green | Green | Red | OFF | 5 s |
| NS yellow | Yellow | Red | OFF | 2 s |
| EW green | Red | Green | OFF | 5 s |
| EW yellow | Red | Yellow | OFF | 2 s |
| Pedestrian | Red | Red | ON | 4 s |

All other LEDs are OFF. The first four states repeat in order. A button request from a vehicle state enters pedestrian mode on the next loop evaluation. Extra requests during WALK are discarded without extending the timer. After 4 seconds, the controller returns to NS green.

The interrupt handler records a flag; the main loop applies the state and updates the LEDs. Serial Monitor prints state changes at 9600 baud.

## Components and wiring

Arduino Uno or compatible board, breadboard, two red LEDs, two yellow LEDs, two green LEDs, one white LED, seven current-limiting resistors (220 ohms as shown), one momentary pushbutton, jumper wires, and USB cable.

| Pin | Connection |
|---|---|
| D4 | NS red LED through resistor |
| D5 | NS yellow LED through resistor |
| D6 | NS green LED through resistor |
| D7 | EW red LED through resistor |
| D8 | EW yellow LED through resistor |
| D9 | EW green LED through resistor |
| D10 | White WALK LED through resistor |
| D2 | Button terminal |
| GND | LED cathodes and other button terminal |

Disconnect power before wiring. Each LED needs its own resistor. The button uses `INPUT_PULLUP` and a `FALLING` interrupt: released is HIGH and pressed is LOW.

**Schematic note:** the supplied schematic appears to join D10 to grounded vehicle LED cathodes. D10 must connect only through the WALK LED branch. Follow the pin table and breadboard layout, and correct the schematic junctions before using it as a construction reference.

## Setup and checks

1. Assemble the circuit and check LED polarity and pin assignments.
2. Save the complete sketch below as `Traffic_Signal_3.ino` in a folder named `Traffic_Signal_3`. The R1 report also contains the final sketch.
3. In Arduino IDE, select Arduino Uno and the connected port, verify, then upload.
4. Open Serial Monitor at 9600 baud. Expect startup text and `State: NS_GREEN`.
5. Check the normal 5 s / 2 s / 5 s / 2 s cycle.
6. Press the button during each vehicle phase. Check both red LEDs and WALK activate for approximately 4 seconds, then NS green resumes.
7. Press again during WALK and check the timer does not restart.

## Submission files
- [Project 5_Traffic Light_First LED Set Image.jpeg](Project%205_Traffic%20Light_First%20LED%20Set%20Image.jpeg)
- [Project 5_Traffic Light_First LED Set Video.mp4](Project%205_Traffic%20Light_First%20LED%20Set%20Video.mp4)
- [Project 5_Traffic Light_R1.docx](Project%205_Traffic%20Light_R1.docx)
- [Project 5_Traffic Light_State Transition.pptx](Project%205_Traffic%20Light_State%20Transition.pptx)
- [Project 5_Traffic Light_State Transition_Diagram.png](Project%205_Traffic%20Light_State%20Transition_Diagram.png)
- [Project 5_Traffic Light_Traffic Signal Image.jpeg](Project%205_Traffic%20Light_Traffic%20Signal%20Image.jpeg)
- [Project 5_Traffic Light_Traffic Signal Video.mp4](Project%205_Traffic%20Light_Traffic%20Signal%20Video.mp4)
- [Project 5_Traffic Light_Traffic Signal_Breadboard.png](Project%205_Traffic%20Light_Traffic%20Signal_Breadboard.png)
- [Project 5_Traffic Light_Traffic Signal_Schema.png](Project%205_Traffic%20Light_Traffic%20Signal_Schema.png)
- [Project 5_Traffic Light_Two LED Set Image.jpeg](Project%205_Traffic%20Light_Two%20LED%20Set%20Image.jpeg)
- [Project 5_Traffic Light_Two LED Set Video.mp4](Project%205_Traffic%20Light_Two%20LED%20Set%20Video.mp4)

The first LED set and two LED set media document earlier build stages. Traffic signal media document the final assembly. Videos are provided as file links for viewing or downloading.

## Images

![Final assembly](Project%205_Traffic%20Light_Traffic%20Signal%20Image.jpeg)

![Breadboard layout](Project%205_Traffic%20Light_Traffic%20Signal_Breadboard.png)

![State transition diagram](Project%205_Traffic%20Light_State%20Transition_Diagram.png)

The directional arrows illustrate an intersection concept. The hardware has two vehicle LED groups and one WALK LED; separate turn-arrow outputs are not implemented.

## Complete sketch

This matches the local final sketch with comment-only and blank lines omitted. No external libraries are required.

```cpp
const int NS_RED    = 4;
const int NS_YELLOW = 5;
const int NS_GREEN  = 6;
const int EW_RED    = 7;
const int EW_YELLOW = 8;
const int EW_GREEN  = 9;
const int WALK_LED = 10;
const int PED_BTN  = 2;     // Pin 2 supports hardware interrupts
enum TrafficState {
  NS_GREEN_STATE,       // State 1
  NS_YELLOW_STATE,      // State 2
  EW_GREEN_STATE,       // State 3
  EW_YELLOW_STATE,      // State 4
  PEDESTRIAN_STATE      // State 5
};
TrafficState currentState = NS_GREEN_STATE;
const unsigned long GREEN_DURATION  = 5000;   // 5 seconds
const unsigned long YELLOW_DURATION = 2000;   // 2 seconds
const unsigned long PED_DURATION    = 4000;   // 4 seconds
unsigned long stateStartTime = 0;
volatile bool pedRequest = false;
void pedestrianRequest() {
  pedRequest = true;
}
void allOff() {
  digitalWrite(NS_RED, LOW);
  digitalWrite(NS_YELLOW, LOW);
  digitalWrite(NS_GREEN, LOW);
  digitalWrite(EW_RED, LOW);
  digitalWrite(EW_YELLOW, LOW);
  digitalWrite(EW_GREEN, LOW);
  digitalWrite(WALK_LED, LOW);
}
void applyState(TrafficState state) {
  allOff();
  switch (state) {
    case NS_GREEN_STATE:
      digitalWrite(NS_GREEN, HIGH);
      digitalWrite(EW_RED, HIGH);
      break;
    case NS_YELLOW_STATE:
      digitalWrite(NS_YELLOW, HIGH);
      digitalWrite(EW_RED, HIGH);
      break;
    case EW_GREEN_STATE:
      digitalWrite(EW_GREEN, HIGH);
      digitalWrite(NS_RED, HIGH);
      break;
    case EW_YELLOW_STATE:
      digitalWrite(EW_YELLOW, HIGH);
      digitalWrite(NS_RED, HIGH);
      break;
    case PEDESTRIAN_STATE:
      digitalWrite(NS_RED, HIGH);
      digitalWrite(EW_RED, HIGH);
      digitalWrite(WALK_LED, HIGH);
      break;
  }
}
TrafficState nextState(TrafficState state, unsigned long elapsed) {
  if (state == PEDESTRIAN_STATE) {
    pedRequest = false;
    if (elapsed >= PED_DURATION) {
      return NS_GREEN_STATE;
    }
    return PEDESTRIAN_STATE;
  }
  if (pedRequest) {
    pedRequest = false;
    return PEDESTRIAN_STATE;
  }
  switch (state) {
    case NS_GREEN_STATE:
      if (elapsed >= GREEN_DURATION) {
        return NS_YELLOW_STATE;
      }
      break;
    case NS_YELLOW_STATE:
      if (elapsed >= YELLOW_DURATION) {
        return EW_GREEN_STATE;
      }
      break;
    case EW_GREEN_STATE:
      if (elapsed >= GREEN_DURATION) {
        return EW_YELLOW_STATE;
      }
      break;
    case EW_YELLOW_STATE:
      if (elapsed >= YELLOW_DURATION) {
        return NS_GREEN_STATE;
      }
      break;
    case PEDESTRIAN_STATE:
      break;
  }
  return state;
}
void setup() {
  pinMode(NS_RED, OUTPUT);
  pinMode(NS_YELLOW, OUTPUT);
  pinMode(NS_GREEN, OUTPUT);
  pinMode(EW_RED, OUTPUT);
  pinMode(EW_YELLOW, OUTPUT);
  pinMode(EW_GREEN, OUTPUT);
  pinMode(WALK_LED, OUTPUT);
  pinMode(PED_BTN, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(PED_BTN),
    pedestrianRequest,
    FALLING
  );
  Serial.begin(9600);
  currentState = NS_GREEN_STATE;
  stateStartTime = millis();
  applyState(currentState);
  Serial.println("Smart Traffic Light Started");
  Serial.println("State: NS_GREEN");
}
void loop() {
  unsigned long elapsed =
      millis() - stateStartTime;
  TrafficState next =
      nextState(currentState, elapsed);
  if (next != currentState) {
    currentState = next;
    stateStartTime = millis();
    applyState(currentState);
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
}
```

## Troubleshooting and limitations

- If an LED stays off, check polarity, resistor, breadboard row, and pin assignment.
- If the button does not respond, check D2-to-GND wiring and common ground.
- If serial text is unreadable, select 9600 baud.
- There is no explicit button debounce. Ignoring requests during WALK is a lockout, not general debounce.
- `volatile` does not make the request check-and-clear sequence atomic. A future revision could protect that sequence briefly from interrupts.
- Durations are nominal; loop execution and serial output can affect observed transition times.
- This is an educational model. Pedestrian requests bypass yellow clearance, and no all-red clearance or fault monitoring is implemented for real traffic control.

## GitHub upload guidance

Suggested repository name: `arduino-traffic-light-controller`

Suggested description:

> Arduino Uno traffic-light controller using a finite state machine, millis() timing, and an interrupt-driven pedestrian button, with report, wiring diagrams, and demonstration videos.

1. Create a repository or open the destination folder in your existing Arduino repository.
2. Choose **Add file > Upload files**. Upload this README and the submission files into the same folder, keeping filenames unchanged.
3. Optionally add the executable sketch as `Traffic_Signal_3/Traffic_Signal_3.ino`, preserving the matching folder and sketch names.
4. Commit summary: `Add traffic light controller documentation`.
5. Optional extended description: `Include R1 report, FSM diagram, wiring, build photos, and demonstration videos.`
6. After committing, open the README and verify images and file links.

All current submission files are below GitHub's 25 MiB per-file browser upload limit. See [GitHub file size guidance](https://docs.github.com/en/repositories/working-with-files/managing-large-files/about-large-files-on-github).
