#include <AccelStepper.h>
#include <AccelStepperWithDistance.h>

// Pin assignments
#define STEPX_PIN 2
#define DIRX_PIN 5
#define STEPY_PIN 3
#define DIRY_PIN 6

char probeHolder[] = "Angled";  // Options: "Angled" or "Vertical"
float probeHolderX;
float probeHolderY;

// Grid parameters
float offsetX = 0;
float offsetY = 0;
int row = 0;
int col = 0;
float colDis = 0;
float rowDis = 0;
unsigned long moveDelay = 2000;  // Default delay (ms)

int startRow = 1, startCol = 1;
int endRow = 1, endCol = 1;

// Home positions
float homex = 0;
float homey = 0;

// Starting well position
float well1X = 0;
float well1Y = 0;

// Stepper setup
AccelStepperWithDistance stepperX(AccelStepperWithDistance::DRIVER, STEPX_PIN, DIRX_PIN); // probe
AccelStepperWithDistance stepperY(AccelStepperWithDistance::DRIVER, STEPY_PIN, DIRY_PIN); // plate

// Scan state
bool scanning = false;
bool initialized = false;
bool moving = false;
bool returningHome = false;
int currentRow = 1;
int currentCol = 1;
bool directionRight = true;
unsigned long lastMoveTime = 0;
unsigned long scanStartTime = 0;
unsigned long scanEndTime = 0;
unsigned long moveStartTime = 0;

// Setup
void setup() {
  Serial.begin(9600);

  // Set probe offsets - determined using OnShape and real life measurements
  if (strcmp(probeHolder, "Angled") == 0) {
    probeHolderX = 0.394;
    probeHolderY = -98.862 + 3.5;
  } else if (strcmp(probeHolder, "Vertical") == 0) {
    probeHolderX = 0.046;
    probeHolderY = -99.427 + 3.5;
  } 
  // Add custom probe holders here
  else {
    probeHolderX = 0.0;
    probeHolderY = 0.0;
    Serial.println("Unknown probe holder type!");
  }

  // Stepper config
  stepperX.setMaxSpeed(1000);
  stepperX.setAcceleration(1000);
  stepperX.setStepsPerRotation(200);
  stepperX.setMicroStep(2);
  stepperX.setDistancePerRotation(40);

  stepperY.setMaxSpeed(2500);
  stepperY.setAcceleration(2500);
  stepperY.setStepsPerRotation(200);
  stepperY.setMicroStep(2);
  stepperY.setDistancePerRotation(8);
}

void loop() {
  handleSerial();  // Listen for scan command
  scan();      // Non-blocking motion logic
  returnHomeCheck();

  // Must be called to allow steppers to move
  stepperX.run();
  stepperY.run();
}

void handleSerial() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.startsWith("scan_all")) {
      int valuesStart = input.indexOf(',') + 1;
      String valuesStr = input.substring(valuesStart);
      float params[7];
      int paramIndex = 0;

      while (paramIndex < 7) {
        int commaIndex = valuesStr.indexOf(',');
        String part;
        if (commaIndex != -1) {
          part = valuesStr.substring(0, commaIndex);
          valuesStr = valuesStr.substring(commaIndex + 1);
        } else {
          part = valuesStr;
        }
        params[paramIndex++] = part.toFloat();
      }

      // Assign
      offsetX    = params[0];
      offsetY    = params[1];
      colDis     = params[2];
      rowDis     = params[3];
      row        = int(params[4]);
      col        = int(params[5]);
      moveDelay  = int(params[6]);

      // Set startRow and startCol to default (1,1)
      startRow = 1;
      startCol = 1;

      // Set endRow and endCol to the max row and column
      endRow = row;
      endCol = col;

      // Starting well X calculation
      well1X = probeHolderX + offsetX + (startCol - 1) * colDis;
      well1Y = probeHolderY + offsetY + (row - startRow) * rowDis;

      // Debug output
      Serial.println("=== Scan Params ===");
      Serial.print("Start Row: "); Serial.println(startRow);
      Serial.print("Start Col: "); Serial.println(startCol);
      Serial.print("End Row: "); Serial.println(endRow);
      Serial.print("End Col: "); Serial.println(endCol);
      Serial.print("Offset X: "); Serial.println(offsetX);
      Serial.print("Offset Y: "); Serial.println(offsetY);
      Serial.print("ColDist:  "); Serial.println(colDis);
      Serial.print("RowDist:  "); Serial.println(rowDis);
      Serial.print("Rows:     "); Serial.println(row);
      Serial.print("Cols:     "); Serial.println(col);
      Serial.print("Delay:    "); Serial.println(moveDelay);

      // Start scan
      scanning = true;
      initialized = false;
      moving = false;
      currentRow = startRow;
      currentCol = startCol;
      directionRight = true;
      returningHome = false;
      scanStartTime = millis();
    }

    // Handle scan_select with specific arguments
    else if (input.startsWith("scan_select")) {
      // Extract parameters
      int valuesStart = input.indexOf(',') + 1;
      String valuesStr = input.substring(valuesStart);
      String parts[10];
      int index = 0;

      // Parse comma-separated values
      while (index < 10 && valuesStr.length() > 0) {
        int commaIndex = valuesStr.indexOf(',');
        if (commaIndex == -1) {
          parts[index++] = valuesStr;
          break;
        } else {
          parts[index++] = valuesStr.substring(0, commaIndex);
          valuesStr = valuesStr.substring(commaIndex + 1);
        }
      }

      // Parse scan_select parameters
      offsetX = parts[0].toFloat();
      offsetY = parts[1].toFloat();
      colDis = parts[2].toFloat();
      rowDis = parts[3].toFloat();
      row = parts[4].toInt();
      col = parts[5].toInt();
      moveDelay = parts[6].toInt();

      // Parse start_well and end_well (e.g., A1, D6)
      String startWell = parts[7];
      String endWell = parts[8];

      startRow = startWell.charAt(0) - 'A'+1;  // Convert letter to row (A = 1, B = 2, ...)
      startCol = startWell.charAt(1) - '0';  // Convert number to column
      endRow = endWell.charAt(0) - 'A' + 1; 
      endCol = endWell.charAt(1) - '0';  

      // Calculate starting well X position
      well1X = probeHolderX + offsetX + (startCol - 1) * colDis;
      well1Y = probeHolderY + offsetY + (row - startRow) * rowDis;

      // Debug output
      Serial.println("=== Scan Select Params ===");
      Serial.print("Start Well: "); Serial.print(startWell);
      Serial.print(" End Well: "); Serial.println(endWell);
      Serial.print("Offset X: "); Serial.println(offsetX);
      Serial.print("Offset Y: "); Serial.println(offsetY);
      Serial.print("ColDist:  "); Serial.println(colDis);
      Serial.print("RowDist:  "); Serial.println(rowDis);
      Serial.print("Rows:     "); Serial.println(row);
      Serial.print("Cols:     "); Serial.println(col);
      Serial.print("Delay:    "); Serial.println(moveDelay);

      // Start scan
      scanning = true;
      initialized = false;
      moving = false;
      currentRow = startRow;
      currentCol = startCol;
      directionRight = true;
      returningHome = false;
      scanStartTime = millis();
    }
  }
}

void scan() {
  if (!scanning) return;

  unsigned long now = millis();

  // Initialization — move to the first well
  if (!initialized) {
    stepperX.moveToDistance(well1X);
    stepperY.moveToDistance(well1Y);

    char rowLetter = 'A' + currentRow - 1;
    Serial.print("Well: ");
    Serial.print(rowLetter);
    Serial.println(currentCol);

    initialized = true;
    moving = true;
    moveStartTime = now;
    return;
  }

  // Wait for movement to finish
  if (moving) {
    if (!stepperX.isRunning() && !stepperY.isRunning()) {
      // Wait for total interval (motor + pause)
      if (now - moveStartTime >= moveDelay) {
        moving = false;
      } else {
        return; 
      }
    } else {
      return;  
    }
  }

  // Stop scanning when currentRow or currentCol exceeds the end row/column
  if (currentRow > endRow || currentCol > endCol) {
    returningHome = true;
    scanning = false;
    stepperX.moveToDistance(homex);
    stepperY.moveToDistance(homey);
    Serial.println("Scanning completed, returning home.");
    return;
  }

  // Move to next column (serpentine direction)
  if ((directionRight && currentCol < endCol) || (!directionRight && currentCol > startCol)) {
    float moveX = directionRight ? colDis : -colDis;
    stepperX.runRelative(moveX);
    currentCol += directionRight ? 1 : -1;
    moving = true;
    moveStartTime = now;

    char rowLetter = 'A' + currentRow - 1;
    Serial.print("Well: ");
    Serial.print(rowLetter);
    Serial.println(currentCol);
    return;
  }

  // Move to next row
  if (currentRow < endRow) {
    stepperY.runRelative(-rowDis);
    currentRow++;

    char rowLetter = 'A' + currentRow - 1;
    Serial.print("Well: ");
    Serial.print(rowLetter);
    Serial.println(currentCol);

    directionRight = !directionRight;
    moving = true;
    moveStartTime = now;
  } else {
    currentRow++;
  }
}

void returnHomeCheck() {
  if (returningHome && !stepperX.isRunning() && !stepperY.isRunning()) {
    scanEndTime = millis();
    Serial.print("Total time (ms): ");
    Serial.println(scanEndTime - scanStartTime);
    returningHome = false;
    Serial.println("Scan completed");
  }
}
