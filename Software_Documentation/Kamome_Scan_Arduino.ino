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
int totalWellsToScan = 0;
int wellsScanned = 0;

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
  stepperX.setMaxSpeed(2000);
  stepperX.setAcceleration(2000);
  stepperX.setStepsPerRotation(200);
  stepperX.setMicroStep(2);
  stepperX.setDistancePerRotation(40);

  stepperY.setMaxSpeed(3000);
  stepperY.setAcceleration(3000);
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

    if (input.startsWith("scan_select")) {
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


      startRow = toupper(startWell.charAt(0)) - 'A' + 1;
      startCol = startWell.substring(1).toInt();

      endRow = toupper(endWell.charAt(0)) - 'A' + 1;
      endCol = endWell.substring(1).toInt();


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

    // Initialize counters
    wellsScanned = 1; // first well is being visited now

    // Compute total wells to scan based on range
    totalWellsToScan = abs(endRow - startRow) + 1;
    totalWellsToScan *= abs(endCol - startCol) + 1;

    return;
  }

  // Wait for movement to finish
  if (moving) {
    if (!stepperX.isRunning() && !stepperY.isRunning()) {
      if (now - moveStartTime >= moveDelay) {
        moving = false;
      } else {
        return; 
      }
    } else {
      return;  
    }
  }

  // NEW END CONDITION: check if we've scanned enough wells
  if (wellsScanned >= totalWellsToScan) {
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
    wellsScanned++;  // increment scan counter

    char rowLetter = 'A' + currentRow - 1;
    Serial.print("Well: ");
    Serial.print(rowLetter);
    Serial.println(currentCol);

    moving = true;
    moveStartTime = now;
    return;
  }

  // Move to next row
  if (currentRow < endRow) {
    stepperY.runRelative(-rowDis);
    currentRow++;

    directionRight = !directionRight;
    wellsScanned++;  // increment scan counter

    char rowLetter = 'A' + currentRow - 1;
    Serial.print("Well: ");
    Serial.print(rowLetter);
    Serial.println(currentCol);

    moving = true;
    moveStartTime = now;
  } else {
    currentRow++; // failsafe
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
