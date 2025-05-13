#include <AccelStepper.h>
#include <AccelStepperWithDistance.h>

#define STEPX_PIN 2
#define DIRX_PIN 5
#define STEPY_PIN 3
#define DIRY_PIN 6

AccelStepperWithDistance stepperX(AccelStepperWithDistance::DRIVER, STEPX_PIN, DIRX_PIN); // moves the probe
AccelStepperWithDistance stepperY(AccelStepperWithDistance::DRIVER, STEPY_PIN, DIRY_PIN); // moves the plate

// User define number of calibration batches: 
const int batches = 40;            // Number of batches

unsigned long prevMillis = 0;  // Store the last time a movement was performed
const long interval = 500;     // Interval for the 500ms wait between steps

// StepperX
int currCycleX = 0;
int currBatchX = 0;
const int cyclesX = 10;            // Number of cycles per batch for belt (10 steps per batch)

bool movingForwardX = true; // Flag to track direction for X

// StepperY
int currCycleY = 0;
int currBatchY = 0;
const int cyclesY = 10;             // Number of cycles per batch for lead screw (10 steps per batch)

bool movingForwardY = false; // Flag to track direction for Y (opposite of X)

void setup() {
  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(1000);
  stepperX.setStepsPerRotation(200);   // For 1.8° stepper motor
  stepperX.setMicroStep(2);            // using half steps
  stepperX.setDistancePerRotation(40); // one rotation of pulley moves 40 mm
  
  stepperY.setMaxSpeed(2900);
  stepperY.setAcceleration(2700);
  stepperY.setStepsPerRotation(200);  // For a 1.8° stepper motor
  stepperY.setMicroStep(2);           //
  stepperY.setDistancePerRotation(8); // one rotation of lead screw moves 8 mm
}

void loop() {
  unsigned long currMillis = millis(); // Get the current time

  // StepperX movements
  if (currBatchX < batches) {

    // Loop through each cycle for X (10 cycles per batch, 90 mm total travel)
    if (currCycleX < cyclesX) {
      if (currMillis - prevMillis >= interval) {
        // Move the motor in the current direction
        if (movingForwardX) {
          stepperX.runRelative(9);  // Move forward 9mm
        } else {
          stepperX.runRelative(-9); // Move backward 9mm if forward is false
        }

        prevMillis = currMillis;  // Reset the last move time
        currCycleX++;  // Move to the next cycle for X
      }
    } else {
      // Once all cycles are completed for X in the batch, move to the next batch
      currCycleX = 0;  // Reset the cycle counter for X

      // Toggle the direction after each batch for X so the probe returns home
      movingForwardX = !movingForwardX;

      // Move to the next batch for X
      currBatchX++;
    }
  } else {
    // Once all batches are completed for stepperX, start stepperY
    if (currBatchY < batches) {
      // Loop through each cycle for Y (10 cycles per batch, 80 mm total travel)
      if (currCycleY < cyclesY) {
        if (currMillis - prevMillis >= interval) {
          if (movingForwardY) {
            stepperY.runRelative(8); 
          } else {
            stepperY.runRelative(-8); 
          }
          prevMillis = currMillis;  
          currCycleY++; 
        }
      } else {
        currCycleY = 0; 
        movingForwardY = !movingForwardY;
        currBatchY++;
      }
    } else {
      return;
    }
  }
}
