const int LED_PIN = LED_BUILTIN;

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(9600);

  // Wait a moment after boot
  delay(2000);

  Serial.println("ARDUINO_READY");
}

void loop() {

  // Check if serial data arrived
  if (Serial.available()) {

    digitalWrite(LED_PIN, HIGH);

    String msg = Serial.readStringUntil('\n');

    msg.trim();

    Serial.print("RECEIVED: ");
    Serial.println(msg);

    // Ping-pong test
    if (msg == "PING") {
      Serial.println("PONG");
    }

    digitalWrite(LED_PIN, LOW);
  }
}