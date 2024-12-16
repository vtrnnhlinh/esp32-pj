#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

#define DHT11_PIN 23                      // ESP32 pin GPIO23 connected to DHT11 sensor
#define RED_PIN   15                      // Pin Red of RGB
#define GREEN_PIN 16                      // Pin Green of RGB
#define BLUE_PIN  17                      // Pin Blue of RGB

const char* ssid = "Your-Wifi-Hotspot";           // Replace with your Wi-Fi SSID
const char* password = "password";   // Replace with your Wi-Fi password

LiquidCrystal_I2C lcd(0x27, 16, 2);       // I2C address 0x27, 16x2 LCD
DHT dht11(DHT11_PIN, DHT11);
WebServer server(80);                     // Create a web server on port 80

float temperature = 0.0;
float humidity = 0.0;

void setColor(bool red, bool green, bool blue) {
  digitalWrite(RED_PIN, red ? LOW : HIGH);
  digitalWrite(GREEN_PIN, green ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blue ? LOW : HIGH);
}

void flashColor(bool red, bool green, bool blue, int delayTime) {
  for (int i = 0; i < 5; i++) {
    setColor(red, green, blue);
    delay(delayTime);
    setColor(HIGH, HIGH, HIGH);
    delay(delayTime);
  }
}

void handleRoot() {
  String html = "<html><head><title>ESP32 Weather</title></head><body>";
  html += "<h1>ESP32 Temperature and Humidity</h1>";
  if (isnan(temperature) || isnan(humidity)) {
    html += "<p>Failed to read from DHT sensor</p>";
  } else {
    html += "<p>Temperature: " + String(temperature) + " °C</p>";
    html += "<p>Humidity: " + String(humidity) + " %</p>";
  }
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(9600);

  // Initialize DHT sensor
  dht11.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  Serial.println(WiFi.localIP());

  // Start Web Server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Read DHT11 sensor
  humidity = dht11.readHumidity();
  temperature = dht11.readTemperature();

  lcd.clear();
  if (isnan(temperature) || isnan(humidity)) {
    lcd.setCursor(0, 0);
    lcd.print("Failed");
    setColor(false, false, false); // OFF ALL LED
    Serial.println("Failed to read from DHT sensor");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Nhiet do: ");
    lcd.print(temperature);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Do am:    ");
    lcd.print(humidity);
    lcd.print("%");

    if (temperature < 25) {
      setColor(HIGH, HIGH, LOW); // (blue)
      Serial.println("Temperature too cold");
    } else if (temperature > 30) {
      flashColor(LOW, HIGH, HIGH, 200);
      setColor(LOW, HIGH, HIGH); // (red)
      Serial.println("Temperature too hot");
    } else {
      setColor(HIGH, LOW, HIGH); // (green)
      Serial.println("Temperature normal");
    }
  }

  // Handle web server requests
  server.handleClient();

  delay(3000); // Wait 3 seconds between readings
}
