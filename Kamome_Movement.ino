#include <AccelStepper.h>
#include <AccelStepperWithDistance.h>
#define STEPX_PIN 2
#define DIRX_PIN 5
#define STEPY_PIN 3
#define DIRY_PIN 6

// Using 96 well
#define row 8
#define col 12
AccelStepperWithDistance stepperX(AccelStepperWithDistance::DRIVER, STEPX_PIN, DIRX_PIN); // moves the probe
AccelStepperWithDistance stepperY(AccelStepperWithDistance::DRIVER, STEPY_PIN, DIRY_PIN); // moves the plate
// Fixed starting positions in millimeters (change these as needed)
float homex = 0;
float homey = 0;
float well1X = 0.394 + 14.38;  // Starting position for X (in mm)
float well1Y = - 98.862 + 11.23;  // Starting position for Y (in mm)
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
  // Define the distances for each row and column
  // 12 well offset = 18.72x 15.94y      col = 30.95 row = 27.95
  // 24 well offset = 17.48x 13.80y      col = 19.30 row = 19.30
  // 48 well offset = 18.16x 10.12y      col = 13.08 row = 13.08
  // 96 well offset = 14.38x 11.23y      col = 9.00 row = 9.00
  float colDis = 9;    // mm distance between columns
  float rowDis = 9;     // mm distance between rows
  //delay(500);
  stepperX.runRelative(well1X);  // Move to the first well position
  stepperY.runRelative(well1Y);
  //stepperX.run();
  //stepperY.run();
  delay(1000);
  // Loop through the grid (3 rows x 4 columns)
  for (int r = 1; r <= row; r++) {
    Serial.print(r);
    // reset X position to the first column
    //stepperX.runToNewDistance(well1X);
    // reset Y position to the rth row
    if (r!=1){
      stepperY.runRelative(rowDis);
     delay(1000);
    } // 2 second delay
    //row number
    for (int c = 1; c < col; c++) {
      Serial.print(c);
      // Subsequent moves: Use runRelative for relative movement
      // if row isn't odd (2nd, 4th, 6th etc), then move the X backwards
      if(r%2 == 1){
        stepperX.runRelative(colDis);
        delay(1000);
      } else { // otherwise move it forwards
        stepperX.runRelative(-colDis);
        delay(1000);
      }
    }
  }
  // After completing all scans, return the motors to the original starting position (1,1)
  stepperX.moveToDistance(homex);  // Move back to the starting X position
  stepperY.moveToDistance(homey);  // Move back to the starting Y position
  stepperX.run();
  stepperY.run();
  //stay at last position
  while (true) {
    stepperX.run();
    stepperY.run();
  }
}