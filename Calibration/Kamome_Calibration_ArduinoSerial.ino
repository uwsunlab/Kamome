#include <AccelStepper.h>
#include <AccelStepperWithDistance.h>

#define STEPX_PIN 2
#define DIRX_PIN 5
#define STEPY_PIN 3
#define DIRY_PIN 6

AccelStepperWithDistance stepperX(AccelStepperWithDistance::DRIVER, STEPX_PIN, DIRX_PIN);
AccelStepperWithDistance stepperY(AccelStepperWithDistance::DRIVER, STEPY_PIN, DIRY_PIN);

// StepperX
int currCycleX = 0;
int currBatchX = 0;
const int cyclesX = 10;
bool movingForwardX = true;

// StepperY
int currCycleY = 0;
int currBatchY = 0;
const int cyclesY = 10;
bool movingForwardY = false;

// Configurable values
int userBatches = 0;
long userInterval = 0;
bool commandReceived = false;

unsigned long prevMillis = 0;

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
      userBatches = input.substring(commaIndex + 1).toInt();

      currBatchX = currBatchY = 0;
      currCycleX = currCycleY = 0;
      movingForwardX = true;
      movingForwardY = false;
      prevMillis = 0;
      commandReceived = true;

      Serial.println("Received");  // Acknowledge receipt
    }
  }

  if (commandReceived) {
    unsigned long currMillis = millis();

    if (currBatchX < userBatches) {
      if (currCycleX < cyclesX) {
        if (currMillis - prevMillis >= userInterval) {
          stepperX.runRelative(movingForwardX ? 9 : -9);
          prevMillis = currMillis;
          currCycleX++;
        }
      } else {
        currCycleX = 0;
        movingForwardX = !movingForwardX;
        currBatchX++;
      }
    } else if (currBatchY < userBatches) {
      if (currCycleY < cyclesY) {
        if (currMillis - prevMillis >= userInterval) {
          stepperY.runRelative(movingForwardY ? 8 : -8);
          prevMillis = currMillis;
          currCycleY++;
        }
      } else {
        currCycleY = 0;
        movingForwardY = !movingForwardY;
        currBatchY++;
      }
    } else {
      commandReceived = false;  // done
      Serial.println("Completed");
    }
  }
}
