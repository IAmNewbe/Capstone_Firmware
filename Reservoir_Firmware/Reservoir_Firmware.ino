
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6t2Mbr3M4"
#define BLYNK_TEMPLATE_NAME         "Water Level"
#define BLYNK_AUTH_TOKEN            "AYpRfdmrBYz1p1rBuilMdZwWhpMX78Td"

#include <BlynkSimpleEsp32.h>

// START DISPLAY PREQUISITE
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Ganti alamat I2C sesuai dengan hasil scan
#define OLED_ADDR 0x3C
#define LCD_ADDR 0x27

// Inisialisasi LCD dan OLED
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

byte customChar[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111
};

// END DISPLAY PREQUISITE

// START DEFINE GPIO
int pump1 = 5;
int pump2 = 17;
int pump3 = 16;
int pump4 = 4;

int pot_1 = 32;
int pot_2 = 33;
int mode;

const int trig1 = 18;
const int echo1 = 19;

const int trig2 = 23;
const int echo2 = 25;

const int slideButton = 34;
// END DEFINE GPIO

// START MQTT PREQUISITE
unsigned long lastSend;
unsigned long lastOn;

const char* ssid = "crustea";
const char* password = "crustea1234";
const char* mqtt_server = "18.140.254.213";
const int mqtt_port = 1883;
const char* topic = "v1/devices/me/telemetry";

#define TOKEN "TEST"

WiFiClient wifiClient;
PubSubClient client(wifiClient);
// END MQTT PREQUISITE


// START HCSR PREQUISITE
//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

long duration;
float distanceCm;
float distanceInch;
unsigned long previousMillis = 0;
// END HCSR PREQUISITE

// START VARIABLE 
  float Main_Level, Reservoir_Level;
  int PotValue_1, PotValue_2;
  int prev_Main = 0;
  int prev_Reservoir = 0;
  int Adjusted_Main_Level, Adjusted_Reservoir_Level;
  int dump, water;
  int Threshold;
  bool state;
  int statePump2;
  float Soil;
  char status_koneksi[8] = "Offline";
  char MODE[18] = "IOT";
// END  VARIABLE

int Water_Level(int trigPin, int echoPin) {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  delay(1000);
  // lcd.clear();

  return distanceCm;
}
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    strcpy(status_koneksi, "Offline");
    oled.clearDisplay();
    oled.setCursor(4, 0);
    oled.setTextSize(1);
    oled.print("Water Level Control");
    oled.drawLine(0, 9, 127, 9, WHITE);
    Offline_Mode();
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  strcpy(status_koneksi, "Online");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}
void Production() {
 
  Adjusted_Main_Level = map(PotValue_1, 0, 4095, 0, 15);
  Serial.println("Offline Mode");
}

void Adjust_Main_Level() {
  if (Adjusted_Main_Level <= Main_Level) {
      digitalWrite(pump4, LOW);
      digitalWrite(pump1, HIGH);
    } else if (Adjusted_Main_Level >= Main_Level) {
      digitalWrite(pump1, LOW);
      digitalWrite(pump4, HIGH);
    } else {
      digitalWrite(pump1, LOW);
      digitalWrite(pump4, LOW);
    }
}

void Adjust_Pump() {
  
  if (mode < 5) {
    Adjust_Main_Level();
  } else if (mode >= 5 && mode < 10 ) {
    if (Soil <= 50) {
      digitalWrite(pump2, HIGH);
      Adjust_Main_Level();
      statePump2 = 1;
      
    } else {
      statePump2 = 0;
      digitalWrite(statePump2, LOW);
      Adjust_Main_Level();
      // Blynk.virtualWrite(V0, statePump2);
    }
  } else if (mode >= 10) {
    Production();
    Adjust_Main_Level();
  }
}

void Display_Level() {
  lcd.setCursor(0, 0);
  lcd.print("Atur Level : ");
  if (prev_Main != Adjusted_Main_Level || prev_Reservoir != Adjusted_Reservoir_Level) {
    lcd.clear();
  }
  if (mode >= 10 || WiFi.status() != WL_CONNECTED) {
    for (int i = 0; i <= Adjusted_Main_Level; i++){
    lcd.setCursor(i,1);
    lcd.write(byte(0));
      if (i > Adjusted_Main_Level) {
        lcd.clear();
      }
    }

  } else {
    int Int_Main_Level = Adjusted_Main_Level;
    int Int_Reservoir_Level = Adjusted_Reservoir_Level; 
    for (int i = 0; i <= Adjusted_Main_Level; i++){
    lcd.setCursor(i, 1);
    lcd.write(byte(0));
    Serial.print("Main Level : ");
    Serial.println(Int_Main_Level);
    Serial.print("i : ");
    Serial.println(i);
      if (i > Adjusted_Main_Level) {
        lcd.clear();
      }
    }
  }
  prev_Main = Adjusted_Main_Level;
  
}

void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
  }
}

void sendData() {
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 2000) {
    lastSend = millis();
    Blynk.virtualWrite(V3, Main_Level);
    Blynk.virtualWrite(V4, Adjusted_Main_Level);
  }
}

void Read_Inputs() {
  Main_Level = Water_Level(trig1, echo1);
  Main_Level = 15 - Main_Level;
  Reservoir_Level = Water_Level(trig2, echo2);
  Reservoir_Level = 15 - Reservoir_Level;

  Main_Level = constrain(Main_Level, 0, 15);
  Reservoir_Level = constrain(Reservoir_Level, 0, 15);

  PotValue_1 = analogRead(pot_1);
  PotValue_2 = analogRead(pot_2);
  mode = map(PotValue_2, 0, 4095, 0, 15);

  if (mode < 5) {
    strcpy(MODE, "Online Control");
  } else if (mode < 10) {
    strcpy(MODE, "Smart Farming");
  } else {
    strcpy(MODE, "Offline Control");
  }

  state = digitalRead(slideButton);
  oled.clearDisplay();
  oled.setCursor(8, 15);
  oled.setTextSize(1);
  oled.print("Tinggi     Tinggi");
  oled.setCursor(9, 23);
  oled.print("Waduk    Reservoir");
  oled.setCursor(20, 33);
  oled.setTextSize(2);
  oled.print((int)Main_Level);
  oled.print("   ");
  oled.print((int)Reservoir_Level);
  oled.display();
}


BLYNK_WRITE(V4)
{
  Adjusted_Main_Level = param.asInt(); // assigning incoming value from pin V1 to a variable

  Serial.print("Adjusted_Main_Level Slider value is: ");
  Serial.println(Adjusted_Main_Level);
}

BLYNK_WRITE(V0)
{
  water = param.asInt(); // assigning incoming value from pin V1 to a variable

   if (water == 1) {
    digitalWrite(pump2, HIGH);
  } else {
    digitalWrite(pump2, LOW);
  }
  
  Serial.print("water button value is: ");
  Serial.println(water);
}

BLYNK_WRITE(V1)
{
  dump = param.asInt(); // assigning incoming value from pin V1 to a variable
  if (dump == 1) {
    digitalWrite(pump3, HIGH);
  } else {
    digitalWrite(pump3, LOW);
  }
  Serial.print("dump button value is: ");
  Serial.println(dump);
}

BLYNK_WRITE(V2)
{
  Soil = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("soil value is: ");
  Serial.println(Soil);
}

void Offline_Mode() {
  Read_Inputs();
  // Production();
  Adjust_Pump();
  Display_Level();
  // oled.clearDisplay();
  strcpy(status_koneksi, "Offline");
  oled.setCursor(0, 48);
  oled.setTextSize(1);
  oled.print("Status : ");
  oled.print(status_koneksi);
  oled.setCursor(0, 56);
  oled.print("Mode : ");
  oled.print(MODE);
  oled.display();
  // WiFi.reconnect();
}

void Online_Mode() {
  Read_Inputs();
  Serial.println("Run Blynk");
  Blynk.run();
  Adjust_Pump();
  // Test_Pump();
  Display_Level();
  sendData();
  strcpy(status_koneksi, "Online");
  oled.setCursor(0, 48);
  oled.setTextSize(1);
  oled.print("Status : ");
  oled.print(status_koneksi);
  oled.setCursor(0, 56);
  oled.print("Mode : ");
  oled.print(MODE);
  oled.display();
}

void loop() {
  reconnect();
  oled.clearDisplay();
  oled.setCursor(4, 0);
  oled.setTextSize(1);
  oled.print("Water Level Control");
  oled.drawLine(0, 9, 127, 9, WHITE);

  if (mode >= 10 || WiFi.status() != WL_CONNECTED) {
    Offline_Mode();
    Serial.println("MODE OFFLINE");
  } else {
    Online_Mode();
    Serial.println("MODE ONLINE");
  }
  oled.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  pinMode(slideButton, INPUT);
  pinMode(pot_1, INPUT);
  pinMode(pot_2, INPUT);
  pinMode(pump1, OUTPUT);
  pinMode(pump2, OUTPUT);
  pinMode(pump3, OUTPUT);
  pinMode(pump4, OUTPUT);

  lcd.init();
  lcd.createChar(0, customChar);
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("CAPSTONE");
  lcd.setCursor(1,1);
  lcd.print("WATER LEVEL");

  if (!oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  oled.display();
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("CAPSTONE");
  oled.display();
  delay(3500);

  setup_wifi();
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  lcd.clear();
  oled.clearDisplay();
}
