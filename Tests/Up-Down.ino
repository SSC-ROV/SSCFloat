//Up Down limits testing

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

// --- LIMIT SWITCHES ---
const int BUTTON_PIN_1 = 19;  // Lower limit
const int BUTTON_PIN_2 = 18;  // Upper limit

enum MoveRequest {
  MOVE_NONE,
  MOVE_DOWN,
  MOVE_UP
};

volatile MoveRequest requestedMove = MOVE_NONE;
volatile bool motorBusy = false;

void stepMotor() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(motorSpeed);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(motorSpeed);
}

void moveUntilPressed(bool movingDown) {
  digitalWrite(dirPin, movingDown ? HIGH : LOW);

  const int limitPin = movingDown ? BUTTON_PIN_1 : BUTTON_PIN_2;
  while (digitalRead(limitPin) == HIGH) {
    stepMotor();
    ArduinoOTA.handle();
    delay(1);
  }
}

String renderPage() {
  String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;padding:24px;}button{padding:16px 20px;margin:10px;font-size:18px;min-width:180px;}</style></head><body>";
  html += "<h2>SSCFloat Control</h2>";
  html += motorBusy ? "<p>Status: Moving</p>" : "<p>Status: Idle</p>";
  html += "<p><a href='/move?dir=down'><button>Move Down</button></a></p>";
  html += "<p><a href='/move?dir=up'><button>Move Up</button></a></p>";
  html += "</body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", renderPage());
}

void handleMove() {
  if (motorBusy) {
    server.send(200, "text/html", renderPage());
    return;
  }

  if (server.hasArg("dir")) {
    String dir = server.arg("dir");
    if (dir == "down") {
      requestedMove = MOVE_DOWN;
    } else if (dir == "up") {
      requestedMove = MOVE_UP;
    }
  }

  server.send(200, "text/html", renderPage());
}

void setup() {
  Serial.begin(115200);

  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  ArduinoOTA.begin();

  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  if (requestedMove != MOVE_NONE && !motorBusy) {
    motorBusy = true;

    if (requestedMove == MOVE_DOWN) {
      moveUntilPressed(true);
    } else if (requestedMove == MOVE_UP) {
      moveUntilPressed(false);
    }

    requestedMove = MOVE_NONE;
    motorBusy = false;
  }
}