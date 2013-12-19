#include  <SPI.h>		// SPI is needed for WiFly
#include  <WiFly.h>

#include <SoftwareServo.h>

#define  WIFI_SSID  ("arduino-wifi")
#define  WIFI_PASS  ("arduinorc")
#define  SERV_PORT  (4242)
#define  REC_DELAY  (1000)
#define  TIMEOUT  (1000)

#define  DEF_SERVO  (90)
#define  MAX_STEER  (120)
#define  MIN_STEER  (60)
#define  MAX_THROT  (120)
#define  MIN_THROT  (60)
#define  MAX_GOPRO  (120)
#define  MIN_GOPRO  (60)

#define  STEER_PIN  (3)
#define  THROT_PIN  (4)
#define  GOPRO_PIN  (5)

#define  LEDCLIENT  (6)
#define  LEDWIFICO  (7)

SoftwareServo  steering;
SoftwareServo  throttle;
SoftwareServo  gopro;

WiFlyServer  server(SERV_PORT);

void  setup() {
  // Init Servos
  steering.attach(STEER_PIN);
  steering.write(DEF_SERVO);
  throttle.attach(THROT_PIN);
  throttle.write(DEF_SERVO);
  gopro.attach(GOPRO_PIN);
  gopro.write(DEF_SERVO);
  // Init LED
  pinMode(LEDCLIENT, OUTPUT);
  digitalWrite(LEDCLIENT, LOW);
  pinMode(LEDWIFICO, OUTPUT);
  digitalWrite(LEDWIFICO, LOW);
  // Init WiFly (may be long)
  WiFly.begin();
}

void  loop() {
  SoftwareServo::refresh();
  digitalWrite(LEDWIFICO, LOW);
  connectWiFly();
  digitalWrite(LEDWIFICO, HIGH);
  server.begin();
  while (serverLoop());
}

bool  serverLoop() {
  bool  ret = true;
  WiFlyClient client = server.available();

  if (client) {
    digitalWrite(LEDCLIENT, HIGH);
    String buffer = "";
    unsigned long lastGet = millis();

    while (client.connected()) {
      int len = client.available();

      if (client.available() == 0 && (millis() - lastGet) > TIMEOUT) {
        ret = false;
        break;
      } else if (client.available() > 0) {
        lastGet = millis();
        for (int n = 0; n < len; n++) {
          char c = client.readChar();
        
          if (c == '\n') {
            if (buffer.length() > 1)
              parseLine(buffer);
            buffer = "";
            break;
          } else
            buffer += c;
        }
      }
      SoftwareServo::refresh();
    }
    digitalWrite(LEDCLIENT, LOW);
    setSteering(DEF_SERVO);
    setThrottle(DEF_SERVO);
    setGoPro(DEF_SERVO);
    SoftwareServo::refresh();
    client.stop();
  }
  SoftwareServo::refresh();
  return ret;
}

void  parseLine(String line) {
  switch (line[0]) {
    case 'S':
      setSteering(line.substring(1).toInt());
      break;
    case 'T':
      setThrottle(line.substring(1).toInt());
      break;
    case 'C':
      setGoPro(line.substring(1).toInt());
      break;
    default:
      break;
  }
}

void  setGoPro(int pos) {
  if (pos < MIN_GOPRO)
    pos = MIN_GOPRO;
  else if (pos > MAX_GOPRO)
    pos = MAX_GOPRO;
  gopro.write(pos);
}

void  setSteering(int pos) {
  if (pos < MIN_STEER)
    pos = MIN_STEER;
  else if (pos > MAX_STEER)
    pos = MAX_STEER;
  steering.write(pos);
}

void  setThrottle(int speed) {
  if (speed < MIN_THROT)
    speed = MIN_THROT;
  else if (speed > MAX_THROT)
    speed = MAX_THROT;
  throttle.write(speed);
}

void  connectWiFly() {
  while (!WiFly.join(WIFI_SSID, WIFI_PASS, WPA_MODE))
    delay(REC_DELAY);
}
