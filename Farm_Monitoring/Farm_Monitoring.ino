#include <PubSubClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
 /* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6t2Mbr3M4"
#define BLYNK_TEMPLATE_NAME         "Water Level"
#define BLYNK_AUTH_TOKEN            "AYpRfdmrBYz1p1rBuilMdZwWhpMX78Td"

#include <BlynkSimpleEsp32.h>
BlynkTimer timer;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
const int AirValue = 3620;   //you need to replace this value with Value_1
const int WaterValue = 1680;  //you need to replace this value with Value_2
const int SensorPin = 35;
int soilMoistureValue = 0;
int soilmoisturepercent=0;
const char* ssid = "crustea";
const char* password = "crustea1234";


void Display() {
  
  display.setCursor(45,0);  //display display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Soil");
  display.setCursor(20,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");
  
  display.drawLine(0, 36, 127, 36, WHITE);
  display.setCursor(30,40);  //display display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.print(soilmoisturepercent);
  display.println(" %");
  display.display();
  
  delay(250);
  display.clearDisplay();
}

void test(){
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
if(soilmoisturepercent > 100)
{
  Serial.println("100 %");
  
  display.setCursor(45,0);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Soil");
  display.setCursor(20,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");
  
  display.setCursor(30,40);  //oled display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println("100 %");
  display.display();
  
  delay(250);
  display.clearDisplay();
}
else if(soilmoisturepercent <0)
{
  Serial.println("0 %");
  
  display.setCursor(45,0);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Soil");
  display.setCursor(20,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");
  
  display.setCursor(30,40);  //oled display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println("0 %");
  display.display();
 
  delay(250);
  display.clearDisplay();
}
else if(soilmoisturepercent >=0 && soilmoisturepercent <= 100)
{
  Serial.print(soilmoisturepercent);
  Serial.println("%");
  
  display.setCursor(45,0);  //oled display
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Soil");
  display.setCursor(20,15);  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("Moisture");
  
  display.setCursor(30,40);  //oled display
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.println(soilmoisturepercent);
  display.setCursor(70,40);
  display.setTextSize(3);
  display.println(" %");
  display.display();
 
  delay(250);
  display.clearDisplay();
}  
}
void readSensor() {
  soilMoistureValue = analogRead(SensorPin);
  Serial.println(soilMoistureValue);
  Blynk.virtualWrite(V2, soilMoistureValue);
  Display();
}

void sendData(){
  Blynk.virtualWrite(V2, soilMoistureValue);
}

void setup() {
  Serial.begin(115200); // open serial port, set the baud rate to 9600 bps
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //initialize with the I2C addr 0x3C (128x64)
  display.clearDisplay();
  display.display();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("CAPSTONE");
  display.display();
  delay(3500);

  Blynk.begin( BLYNK_AUTH_TOKEN, ssid, password);
  display.clearDisplay();
  // Set a timer to call the readSensor function every second
  // timer.setInterval(1000L, readSensor);
  // timer.setInterval(3000L, readSensor);
}
 
 
void loop() 
{
  Blynk.run();
  soilMoistureValue = analogRead(SensorPin);
  Serial.println(soilMoistureValue);
  soilmoisturepercent = soilMoistureValue * 100 /4096 ;
  Blynk.virtualWrite(V2, soilmoisturepercent);
  Display();
  // delay(1000);
  // timer.run();
}