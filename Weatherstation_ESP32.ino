#define ERA_DEBUG

#define DEFAULT_MQTT_HOST "mqtt1.eoh.io"

// You should get Auth Token in the ERa App or ERa Dashboard
#define ERA_AUTH_TOKEN "YOUR_ERA_TOKEN"

#include <Arduino.h>
#include <ERa.hpp>
#include <ERa/ERaTimer.hpp>

#include <Wire.h>              // Wire library (required for I2C devices)
#include <Adafruit_GFX.h>      // Adafruit core graphics library
#include <SPI.h>
#include <TFT_eSPI.h>     // Adafruit hardware-specific library for ST7789
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Images.h"
#include "DHT.h"
#include <AnimatedGIF.h>
AnimatedGIF gif;
#include "loading.h"

#define GIF_IMAGE loading   

TFT_eSPI tft = TFT_eSPI(); 

const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char host[] = "api.openweathermap.org";
String APIKEY = "YOUR_APIKEY";
String CityID = "CityID";

String data = "";
String result;

// Define NTP Client to get time
WiFiClient client;
WiFiUDP ntpUDP;
unsigned long lastConnectionTime = 10L * 6000L;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 6000L;
NTPClient timeClient(ntpUDP, "2.asia.pool.ntp.org", 7*3600, 60000);

const char* ntpServer = "2.asia.pool.ntp.org";
const long  gmtOffset_sec = 7*3600;
const int   daylightOffset_sec = 3600;

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thusday", "Friday", "Saturday"};

//Month names
String months[12]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

time_t dstOffset = 7*3600;

#define DHTPIN 19

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

ERaTimer timer;

/* This function print uptime every second */
void timerEvent() {
    ERA_LOG("Timer", "Uptime: %d", ERaMillis() / 1000L);
}

TaskHandle_t Era;  

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  //WiFi.begin(ssid, password);
  tft.init();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  tft.setRotation(0);
  tft.fillScreen(TFT_WHITE);

  WiFi.begin(ssid, password);
  gif.begin(BIG_ENDIAN_PIXELS);
  tft.setCursor(38, 245);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(4); 
  tft.print("LOADING");
  while (WiFi.status() != WL_CONNECTED) {
    if (gif.open((uint8_t *)GIF_IMAGE, sizeof(GIF_IMAGE), GIFDraw))
    {
      Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
      tft.startWrite(); // The TFT chip select is locked low
      while (gif.playFrame(true, NULL))
      {
        yield();
      }
      gif.close();
      tft.endWrite(); // Release TFT chip select for other SPI devices
    }
}
  delay(2000);
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes (true); 
  tft.pushImage(4,7,64,64,timetable);
  tft.pushImage(98,290,44,24,EoH);
  tft.drawWideLine(1, 73, 239, 73, 1, TFT_WHITE);
  //tft.drawWideLine(1, 140, 239, 140, 1, TFT_WHITE);
  tft.drawWideLine(1, 283, 239, 283, 1, TFT_WHITE);
  tft.drawRect(0, 0, 240, 320, TFT_WHITE);

  tft.setTextWrap(false); 
  
  dht.begin();

  timeClient.begin();
  timeClient.update();
  xTaskCreatePinnedToCore(Task1_Era,"Era",10000,NULL,1,&Era,1);  delay(500); 

}
// Lay gia tri cam bien
void DHT11_Sensor()
{
  float t = dht.readTemperature();
  //Serial.print(t);
  //Serial.println(" *C");
  //////////////////////////
  float h = dht.readHumidity();
  //Serial.print(p/100);
  //Serial.println(" hPa");
  DisplayDHT(t, h);
  ERa.virtualWrite(V0, t);
  ERa.virtualWrite(V1, h);
}
// Hien thi gia tri cam bien len lcd
void DisplayDHT(float &t, float &h)
{
  tft.setTextColor(TFT_VIOLET, TFT_BLACK);  // set text color to green and black background
  tft.setTextSize(1);       
  tft.setCursor(12, 288);
  tft.print("Temperature");
  tft.setCursor(170, 288); 
  tft.print("Humidity"); 
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(2); 
  tft.setCursor(5, 300);
  tft.print(t); tft.print((char)247); tft.print("C");
  tft.setCursor(158, 300);
  tft.print(h); tft.print("%");
}
// Lay gia tri thoi gian
void Time()
{
  //delay(100);
  //timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  
  String formattedTime = timeClient.getFormattedTime();
  //Serial.print("Formatted Time: ");
  //Serial.println(formattedTime);  

  // int currentHour = timeClient.getHours();
  // Serial.print("Hour: ");
  // Serial.println(currentHour);  

  // int currentMinute = timeClient.getMinutes();
  // Serial.print("Minutes: ");
  // Serial.println(currentMinute); 
   
  // int currentSecond = timeClient.getSeconds();
  // Serial.print("Seconds: ");
  // Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  // // Serial.print("Week Day: ");
  // // Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  // Serial.print("Month day: ");
  // Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  // Serial.print("Month: ");
  // Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  // Serial.print("Month name: ");
  // Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  // Serial.print("Year: ");
  // Serial.println(currentYear);

  //Print complete date:
  // String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  // Serial.print("Current date: ");
  // Serial.println(currentDate);
  // Serial.println("");

  /////////////////////////////////////////////////////////////
  String Date = String(currentMonthName) + " " + String(monthDay) + ", " + String(currentYear);
  //Serial.print("Date: ");
  //Serial.println(Date);
  String Date1 = String(weekDay);
  //Serial.print("Date: ");
  //Serial.println(Date1);
  /////////////////////////////////////////////////////////////
  DisplayTime(Date, Date1, formattedTime);
}
// Hien thi thoi gian len lcd
void DisplayTime(String &Date, String &Date1, String &Time)
{
  /////////////////////////
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);  // set text color to green and black background
  tft.setTextSize(2);       
  tft.setCursor(75, 7);
  tft.print(Date1);   
  ///////////////////////// 
  tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);  // set text color to green and black background
  tft.setTextSize(2);       
  tft.setCursor(75, 27);
  tft.print(Date);
  ///////////////////////////////
  tft.setTextColor(TFT_BLUE, TFT_BLACK);  // set text color to green and black background
  tft.setTextSize(3);       
  tft.setCursor(75, 47);
  tft.print(Time);
}

void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();
  delay(1);

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  const int httpPort = 80;
  client.connect(host, httpPort);

  //Serial.print("Requesting URL: ");
    client.println("GET /data/2.5/weather?id=" + CityID + "&units=metric&APPID=" + APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

  lastConnectionTime = millis();
  delay(1);
}


// Icon thoi tiet
void DisplayIcon(String &icon)
{
  if(icon == "Clear") {
    tft.pushImage(20,80,80,80,sun);
    tft.setTextSize(2);       
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(32, 165);
    tft.printf("Clear");
    }
  if(icon == "Clouds") {
    tft.pushImage(20,80,80,80,clouds);
    tft.setTextSize(2);       
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(25, 165);
    tft.printf("Clouds");
    }
  if(icon == "Rain") {
    tft.pushImage(20,80,80,80,rain);
    tft.setTextSize(2);       
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(35, 165);
    tft.printf("Rain");
    }
  if(icon == "Drizzle") {
    tft.pushImage(20,80,80,80,rain);
    tft.setTextSize(2);       
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(35, 165);
    tft.printf("Rain");
    }
  if(icon == "Thunderstorm") {
    tft.pushImage(20,80,80,80,light_rain_thunder);
    tft.setTextSize(2);       
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setCursor(30, 165);
    tft.printf("Storm");
    }
}

void Weather()
{
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  while (client.connected() && !client.available())
    delay(10);                                          //waits for data
  while (client.connected() || client.available())
  { //connected or data available
    char c = client.read();                     //gets byte from ethernet buffer
    result = result + c;
  }

  //client.stop();                                      //stop client
  result.replace('[', ' ');
  result.replace(']', ' ');
  //Serial.println(result);
  char jsonArray [result.length() + 1];
  result.toCharArray(jsonArray, sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> myObject;
  DeserializationError  error = deserializeJson(myObject, jsonArray);


  String str_current_weather = myObject["weather"]["main"];
  String icon = myObject["weather"]["icon"];
  String Location = myObject["name"];
  String Country = myObject["sys"]["country"];
  // tft.setTextColor(TFT_GREEN, TFT_BLACK);  // set text color to green and black background
  // tft.setTextSize(2);       
  // tft.setCursor(25, 165);
  // tft.print(str_current_weather); 
  time_t now = myObject["sys"]["sunrise"];
    //Serial.println(now);
    struct tm tInfo = *localtime(&now);
    char str[10];
    strftime(str, sizeof(str), "%H:%M", &tInfo);
    //Serial.println(str);

    time_t now1 = myObject["sys"]["sunset"];
    //Serial.println(now1);
    struct tm tInfo1 = *localtime(&now1);
    char str1[10];
    strftime(str1, sizeof(str1), "%H:%M", &tInfo1);
    //Serial.println(str1);

//////////////////////////////////////////
      float temperature = myObject["main"]["temp"];
      tft.setTextColor(TFT_VIOLET, TFT_BLACK);
      tft.setCursor(125, 80);
      tft.setTextSize(1);
      tft.print("Temperature:");
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setCursor(125, 92);
      tft.setTextSize(2);
      tft.print(temperature);
      tft.print((char)247); tft.print("C");
      ERa.virtualWrite(V2, temperature);
/////////////////////////////////////////////////////////
      float humidity = myObject["main"]["humidity"];
      tft.setTextColor(TFT_VIOLET, TFT_BLACK);
      tft.setCursor(125, 110);
      tft.setTextSize(1);
      tft.print("Humidity:");
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(125, 122);
      tft.print(humidity); tft.print("%");
      ERa.virtualWrite(V3, humidity);
//////////////////////////////////////////////////////// 
      int pressure = myObject["main"]["pressure"];
      tft.setTextColor(TFT_VIOLET, TFT_BLACK);
      tft.setCursor(125, 140);
      tft.setTextSize(1);
      tft.print("Pressure:");
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(125, 152);
      tft.print(pressure); tft.print("hPa");
      ERa.virtualWrite(V4, pressure);
 //////////////////////////////////////////         
      tft.setTextColor(TFT_VIOLET, TFT_BLACK);
      float ws = myObject["wind"]["speed"];
      tft.setCursor(125, 170);
      tft.setTextSize(1);
      tft.print("Wind Speed:");
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(2);
      tft.setCursor(125, 182);
      tft.print(ws); tft.print("m/s");
      ERa.virtualWrite(V5, ws);
//////////////////////////////////////////////////////////////
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.setTextSize(1);
      tft.setCursor(55, 275);
      tft.print(Location);  tft.print(" - ");
      //tft.setCursor(168, 275);
      tft.print(Country);
//////////////////////////////////////////////////////////////
      tft.setTextColor(TFT_GREEN, TFT_BLACK);
      tft.setTextSize(2);
      tft.pushImage(30,206,45,45,sunrise);
      tft.setCursor(24, 255);
      tft.print(str);
      tft.pushImage(165,206,45,45,sunset);
      tft.setCursor(159, 255);
      tft.print(str1);

      DisplayIcon(str_current_weather);
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    data = client.readStringUntil('\r');
    //Serial.print(data);
  }
}

void Task1_Era(void * parameter)
{
  Serial.println(xPortGetCoreID()); 
  ERa.begin(ssid, password);
  timer.setInterval(1000L, timerEvent);
  for(;;){
    ERa.run();
    timer.run();
  }
}


void loop() {

    DHT11_Sensor();
    Time();
    if (WiFi.status() == WL_CONNECTED) {
      timeClient.update();
      Weather();
    }
    delay(10);
}

///////////////////////////////////////////////////////////




