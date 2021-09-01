#include <Wire.h>
#include "MAX30100_PulseOximeter.h"
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire);
 
#define REPORTING_PERIOD_MS 1000
 
char auth[] = "Token";             // Auth Token in Blynk App.
char ssid[] = "x";                                     //Hospot Name.
char pass[] = "xxxxxxxx";                                 //Password

//For mean function
float mean[10]; // Sensor unstable
int i = 0;

// Connections : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0
PulseOximeter pox;
 
float BPM = 0.0,SpO2 = 0.0;

//For timer interrupt
uint32_t tsLastReport = 0;

//Blynk LCD
WidgetLCD lcd(V1);

void onBeatDetected()
{
    Serial.println("Beat Detected!");
}
//set up
void setup()
{
    Serial.begin(115200);
    
    //pinMode(19, OUTPUT);
    Blynk.begin(auth, ssid, pass);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c)) 
    {
      Serial.println(F("SSD1306 allocation failed"));
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 15);
    display.println(F("Loading..."));
    display.display();
    lcd.print(3,0,"Waiting...");
    lcd.clear();
    
    Serial.print("Initializing Pulse Oximeter..");
 
    if (!pox.begin())
    {
         Serial.println("FAILED");
         for(;;);
    }
    else
    {
         Serial.println("SUCCESS");
         pox.setOnBeatDetectedCallback(onBeatDetected);
    }
    pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
}

void LCD(float bpm,float spo2){
  lcd.print(0, 0, "Heart Rate: ");
  lcd.print(11, 0, bpm);
  lcd.print(0, 1, "SpO2: ");
  lcd.print(11, 1, spo2);
}

void Mean(){
  float sum = 0.0;
  for(int k = 0 ; k <10 ;k++)
  {
    sum += mean[k];
  }
 //Check
  Serial.println("MEAN: ");
  Serial.print(sum/10.0);
        
  display.clearDisplay();  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 15);
  display.println(F("BMP mean: "));      
  display.print(sum/10.0);
  lcd.clear();
  lcd.print(6, 0, "MEAN");
  lcd.print(3,1,sum/10.00);
  delay(2000);
  lcd.clear();

  if(sum/10.00 < 70.0 or sum/10.00 > 120)
  {
    lcd.print(4,0,"Aleart!!");
  }
  else{
    lcd.print(3,0,"Good Heart!");
  }
  delay(2000);
  display.display();
        
  sum = 0.0;
  i = 0;
}

void loop()
{
    pox.update();
    Blynk.run();
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();
    
    if (millis() - tsLastReport > REPORTING_PERIOD_MS) //REPORTING_PERIOD_MS 1000
    {
      if(BPM > 60.0 and BPM < 120.0)//for reasonable value
      {
        Blynk.virtualWrite(V3, BPM);
        Blynk.virtualWrite(V4, SpO2);

        display.clearDisplay();
        display.setTextSize(1.3);
        display.setTextColor(WHITE);
        display.setCursor(0, 0);
        display.println(F("Heart Rate: "));
        display.setCursor(80, 0);
        display.print(BPM);
        display.setCursor(0, 16);
        display.println(F("SpO2: "));
        display.setCursor(80, 16);
        display.print(SpO2);
        if(i == 0)
        {
          mean[i] = BPM;
          i++;
        }
        if(mean[i-1] != BPM)//duplicate value
        {
          mean[i] = BPM;
          Serial.print(mean[i]);
          i++;
        }
        else{
          pox.shutdown();
          pox.begin();
        }
      }
      Serial.print("Heart rate:");
      Serial.print(BPM);
      Serial.print(" bpm / SpO2:");
      Serial.print(SpO2);
      Serial.println(" %");
      Serial.print("i");
      Serial.print(i);

      lcd.print(3,0,"Waiting...");
      
      
      //mean calculate
      if(i == 10)
      {
        Mean();
        pox.begin();
        i == 0;
        lcd.clear();
      }
      display.display();
      tsLastReport = millis();
    }
}
