#include  <SPI.h>
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

void  loop()
{
  SoftwareServo::refresh();
  digitalWrite(LEDWIFICO, LOW);
  while (!WiFly.join(WIFI_SSID, WIFI_PASS, WPA_MODE))
    delay(REC_DELAY);
  digitalWrite(LEDWIFICO, HIGH);
  server.begin();
  while (serverLoop());
}

bool  serverLoop()
{
  bool  ret = true;
  WiFlyClient client = server.available();

  if (client)
  {
    digitalWrite(LEDCLIENT, HIGH);
    String buffer = "";
    unsigned long lastGet = millis();

    while (client.connected())
    {
      int len = client.available();

      if (len == 0 && (millis() - lastGet) > TIMEOUT)
      {
        ret = false;
        break;
      }
      else if (len > 0)
      {
        lastGet = millis();
        for (int n = 0; n < len; n++)
        {
          char c = client.readChar();
        
          if (c == '\n')
          {
            if (buffer.length() >= 2)
              parseLine(buffer);
            buffer = "";
            break;
          }
          else
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

inline void  parseLine(String &line)
{
  int  pos = line.substring(1).toInt();
  
  switch (line[0])
  {
    case 'S':
      setSteering(pos);
      break;
    case 'T':
      setThrottle(pos);
      break;
    case 'C':
      setGoPro(pos);
      break;
    default:
      break;
  }
}

inline void  setGoPro(int pos)
{
  gopro.write((pos < MIN_GOPRO) ? MIN_GOPRO : ((pos > MAX_GOPRO) ? MAX_GOPRO : pos));
}

inline void  setSteering(int pos)
{
  steering.write((pos < MIN_STEER) ? MIN_STEER : ((pos > MAX_STEER) ? MAX_STEER : pos));
}

inline void  setThrottle(int pos)
{
  throttle.write((pos < MIN_THROT) ? MIN_THROT : ((pos > MAX_THROT) ? MAX_THROT : pos));
}
