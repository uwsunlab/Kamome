#include <AccelStepper.h>
#include <AccelStepperWithDistance.h>

#define STEPX_PIN 2
#define DIRX_PIN 5
#define STEPY_PIN 3
#define DIRY_PIN 6

AccelStepperWithDistance stepperX(AccelStepperWithDistance::DRIVER, STEPX_PIN, DIRX_PIN);
AccelStepperWithDistance stepperY(AccelStepperWithDistance::DRIVER, STEPY_PIN, DIRY_PIN);

// StepperX
int currIncrementX = 0;
int currCycleX = 0;
const int incrementsX = 10;
bool movingForwardX = true;
bool stepperXRunning = false;

// StepperY
int currIncrementY = 0;
int currCycleY = 0;
const int incrementsY = 10;
bool movingForwardY = false;
bool stepperYRunning = false;

// Configurable values
int userCycles = 0;
int userCyclesHalf = 0;
long userInterval = 0;
bool commandReceived = false;

void setup() {
  Serial.begin(9600);

  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(1000);
  stepperX.setStepsPerRotation(200);
  stepperX.setMicroStep(2);
  stepperX.setDistancePerRotation(40);

  stepperY.setMaxSpeed(2900);
  stepperY.setAcceleration(2700);
  stepperY.setStepsPerRotation(200);
  stepperY.setMicroStep(2);
  stepperY.setDistancePerRotation(8);
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    int commaIndex = input.indexOf(',');

    if (commaIndex > 0) {
      userInterval = input.substring(0, commaIndex).toInt();
      userCyclesHalf = input.substring(commaIndex + 1).toInt();
      userCycles = userCyclesHalf*2;

      currCycleX = currCycleY = 0;
      currIncrementX = currIncrementY = 0;
      movingForwardX = true;
      movingForwardY = false;
      stepperXRunning = false;
      stepperYRunning = false;
      commandReceived = true;

      Serial.println("Received");  // Acknowledge receipt
    }
  }

  if (commandReceived) {
    // Handle Stepper X
    if (currCycleX < userCycles) {
      stepperX.run();

      if (!stepperXRunning && currIncrementX < incrementsX) {
        stepperX.runRelative(movingForwardX ? 9 : -9);
        stepperXRunning = true;
      }

      if (stepperXRunning && !stepperX.isRunning()) {
        stepperXRunning = false;
        currIncrementX++;
        //Serial.print("Stepper X increment: ");
        //Serial.println(currIncrementX);
      }

      if (currIncrementX >= incrementsX) {
        currIncrementX = 0;
        movingForwardX = !movingForwardX;
        currCycleX++;
        //Serial.print("Stepper X cycle: ");
        //Serial.println(currCycleX);
      }
    }

    // After Stepper X completes, run Stepper Y
    else if (currCycleY < userCycles) {
      stepperY.run();

      if (!stepperYRunning && currIncrementY < incrementsY) {
        stepperY.runRelative(movingForwardY ? 8 : -8);
        stepperYRunning = true;
      }

      if (stepperYRunning && !stepperY.isRunning()) {
        stepperYRunning = false;
        currIncrementY++;
        //Serial.print("Stepper Y increment: ");
        //Serial.println(currIncrementY);
      }

      if (currIncrementY >= incrementsY) {
        currIncrementY = 0;
        movingForwardY = !movingForwardY;
        currCycleY++;
        //Serial.print("Stepper Y cycle: ");
        //Serial.println(currCycleY);
      }
    }

    // All done
    else {
      commandReceived = false;
      Serial.println("Completed");
    }
  }
}
