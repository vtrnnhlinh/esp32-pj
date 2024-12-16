#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHT11_PIN 23                      // ESP32 pin GPIO23 connected to DHT11 sensor
#define RED_PIN   15                      //Pin Red of rgb
#define GREEN_PIN 16                      //Pin green of rgb
#define BLUE_PIN  17                      //Pin Blue of rgb

LiquidCrystal_I2C lcd(0x27, 16, 2);       // I2C address 0x27 (from DIYables LCD), 16 column and 2 rows
DHT dht11(DHT11_PIN, DHT11);

void setColor(bool red, bool green, bool blue)
{
  // LOW: ON LED, HIGH: OFF LED (common cathode)
  digitalWrite(RED_PIN, red ? LOW : HIGH);   
  digitalWrite(GREEN_PIN, green ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blue ? LOW : HIGH);
}

void flashColor(bool red, bool green, bool blue, int delayTime)
{
  // blinky 
  for (int i = 0; i < 5; i++) 
  {
    setColor(red, green, blue);
    delay(delayTime);
    setColor(HIGH, HIGH, HIGH);
    delay(delayTime);
  }
}
void setup()
{
  /*Initialize serial*/
  Serial.begin(9600);

  /*initialize the DHT11 sensor*/
  dht11.begin();  

  /*initialize the lcd*/
  lcd.init();                               
  lcd.backlight();                          // open the backlight

  /*Initialize for pin led RGB*/
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
}

void loop()
{
  float humidity = dht11.readHumidity();      // read humidity
  float temperature = dht11.readTemperature();// read temperature

  lcd.clear();
  // check whether the reading is successful or not
  if (isnan(temperature) || isnan(humidity))
  {
    lcd.setCursor(0, 0);
    lcd.print("Failed");
    setColor(false, false, false);           // OFF ALL LED
    Serial.println("Failed to read from DHT sensor");
  }
  else
  {
    lcd.setCursor(0, 0);                  // display position
    lcd.print("Nhiet do: ");
    lcd.print(temperature);               // display the temperature
    lcd.print("C");

    lcd.setCursor(0, 1);                  // display position
    lcd.print("Do am:    ");
    lcd.print(humidity);                  // display the humidity
    lcd.print("%");

    if (temperature < 25)
    {
      setColor(HIGH, HIGH, LOW);        // (blue)
      Serial.println("Temperature too cold");
    }
    else if (temperature > 30)
    {
      flashColor(LOW, HIGH, HIGH, 200);
      setColor(LOW, HIGH, HIGH);        //  (red)
      Serial.println("Temperature too hot");
    }
    else
    {
      setColor(HIGH, LOW, HIGH);        // (green)
      Serial.println("Temperature normal");
    }
  }
 
  delay(3000);                           // wait a 2 seconds between readings
}