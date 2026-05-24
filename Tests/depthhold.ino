//Tests for suck in water until depth changes then the stepper motor stop reset back to the float

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

// --- WIFI CREDENTIALS ---
const char* ssid = "SSCFloat RN10";
const char* password = "DT1234dt";
WebServer server(80);

// --- MOTOR PINS & SETTINGS ---
const int dirPin = 4;
const int stepPin = 16;
const int motorSpeed = 600;

// --- BUTTON & PRESSURE SENSOR ---
const int START_BUTTON = 21;     // Optional physical start button
const int PRESSURE_SENSOR = 34;  // Analog input for pressure sensor
const int BUTTON_PIN_1 = 19;     // Lower limit
const int BUTTON_PIN_2 = 18;     // Upper limit

// --- PRESSURE MONITORING ---
const int PRESSURE_THRESHOLD = 50;  // Pressure change threshold to trigger stop
const int SAMPLE_SIZE = 5;          // Number of samples to average
volatile bool motorBusy = false;
volatile bool stopMotor = false;
volatile bool isExpelling = false;

void stepMotor() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motorSpeed);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(motorSpeed);
}

int readAveragePressure() {
  int sum = 0;
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    sum += analogRead(PRESSURE_SENSOR);
    delay(10);
  }
  return sum / SAMPLE_SIZE;
}

void moveDownWithPressureMonitor() {
  digitalWrite(dirPin, HIGH);  // Set direction to DOWN
  
  int initialPressure = readAveragePressure();
  Serial.print("Initial Pressure: ");
  Serial.println(initialPressure);
  
  stopMotor = false;
  
  // Move down until lower limit is hit or pressure changes suddenly
  while (!stopMotor && digitalRead(BUTTON_PIN_1) == HIGH) {
    stepMotor();
    
    // Check pressure periodically (every 50 steps)
    static int stepCount = 0;
    stepCount++;
    
    if (stepCount >= 50) {
      stepCount = 0;
      int currentPressure = readAveragePressure();
      int pressureDelta = abs(currentPressure - initialPressure);
      
      Serial.print("Current Pressure: ");
      Serial.print(currentPressure);
      Serial.print(" | Delta: ");
      Serial.println(pressureDelta);
      
      // Stop if pressure changed suddenly
      if (pressureDelta > PRESSURE_THRESHOLD) {
        Serial.println("Sudden pressure change detected - stopping motor!");
        stopMotor = true;
        break;
      }
    }
    
    ArduinoOTA.handle();
    delay(1);
  }
  
  Serial.println("Motor stopped.");
}

void expelWater() {
  digitalWrite(dirPin, LOW);  // Set direction to UP (expel water)
  
  Serial.println("Expelling water...");
  
  // Move up until upper limit is hit
  while (digitalRead(BUTTON_PIN_2) == HIGH) {
    stepMotor();
    ArduinoOTA.handle();
    delay(1);
  }
  
  Serial.println("Water expelled. Motor at upper limit.");
}

String renderPage() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:24px;}button{padding:16px 20px;margin:10px;font-size:18px;min-width:180px;}</style></head><body>";
  html += "<h2>SSCFloat Control</h2>";
  html += motorBusy ? "<p>Status: Moving</p>" : "<p>Status: Idle</p>";
  html += "<p><a href='/start'><button style='background-color:#4CAF50;color:white;'>Move Down (Fill)</button></a></p>";
  html += "<p><a href='/expel'><button style='background-color:#FF6B6B;color:white;'>Expel Water (Reset)</button></a></p>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", renderPage());
}

void handleWiFiStart() {
  if (!motorBusy) {
    motorBusy = true;
    stopMotor = false;
    isExpelling = false;
  } else {
    stopMotor = true;
  }
  server.send(200, "text/html", renderPage());
}

void handleExpel() {
  if (!motorBusy) {
    motorBusy = true;
    isExpelling = true;
  }
  server.send(200, "text/html", renderPage());
}

void setup() {
  Serial.begin(115200);

  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(PRESSURE_SENSOR, INPUT);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/start", handleWiFiStart);
  server.on("/expel", handleExpel);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (motorBusy) {
    if (isExpelling) {
      expelWater();
      isExpelling = false;
    } else {
      moveDownWithPressureMonitor();
    }
    motorBusy = false;
  }
}