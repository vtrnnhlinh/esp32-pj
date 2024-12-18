#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "ESPAsyncWebServer.h"

#define DHTTYPE DHT11                     // DHT 11
#define DHT11_PIN 23                      // DHT11 sensor connected to pin 23 on ESP32
#define RED_PIN   15                      // RGB LED Red pin
#define GREEN_PIN 16                      // RGB LED Green pin
#define BLUE_PIN  17                      // RGB LED Blue pin

const char* ssid = "Nga.Ntt";            // Wi-Fi SSID
const char* password = "08032004";       // Wi-Fi password

LiquidCrystal_I2C lcd(0x27, 16, 2);      // LCD screen with I2C address 0x27 and size 16x2
DHT dht11(DHT11_PIN, DHT11);             // DHT11 sensor initialization

AsyncWebServer server(80);               // Web server on port 80

/* Timer variables for millis() */
unsigned long previousMillis = 0;        // Store time for tasks that require delay
unsigned long previousFlashMillis = 0;   // Store time for RGB LED flashing
unsigned long lastWiFiCheck = 0;         // Time to check Wi-Fi status

int flashState = 0;                      // State of the flashing RGB LED
int flashCount = 0;                      // Count for the number of LED flashes

const int interval = 2000;               // Update interval for LCD every 2 seconds
const int flashInterval = 200;           // Flash interval for RGB LED every 200ms
const int WiFiCheckInterval = 3000;      // Wi-Fi check interval every 3 seconds

/*SET color for led*/
void setColor(bool red, bool green, bool blue) {
  digitalWrite(RED_PIN, red ? LOW : HIGH);
  digitalWrite(GREEN_PIN, green ? LOW : HIGH);
  digitalWrite(BLUE_PIN, blue ? LOW : HIGH);
}

// Function to flash RGB LED with millis()
void flashColor(bool red, bool green, bool blue, int delayTime) {
  unsigned long currentMillis = millis();

  if (currentMillis - previousFlashMillis >= delayTime) {
    previousFlashMillis = currentMillis;  // Update time

    if (flashState == 0) { 
      flashState = 1;
    } else {
      setColor(HIGH, HIGH, HIGH);  
      flashState = 0;
      flashCount++;  
    }
  }

  // Stop flashing after 10 flashes
  if (flashCount >= 10) {
    flashState = 0;
    flashCount = 0;
    setColor(red, green, blue);           // ON flashed led 
  }
}

// Function to read temperature from DHT sensor
String readDHTTemperature() {
  float t = dht11.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(t);
  }
}

// Function to read humidity from DHT sensor
String readDHTHumidity() {
  float h = dht11.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(h);
  }
}

/*============================HTML template for the web server=========================*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    h2 { 
      font-size: 2.0rem; 
      color: #00008B;
    }
    .box {
      border: 2px solid #ccc;
      border-radius: 10px;
      padding: 20px;
      margin: 20px auto;
      max-width: 300px;
      background-color: #f9f9f9;
      box-shadow: 2px 2px 8px rgba(0, 0, 0, 0.2);
    }
    .box-header {
      display: flex;
      align-items: center;
      justify-content: center;
      margin-bottom: 15px;
    }
    .box-header i {
      font-size: 2.5rem;
      margin-right: 10px;
    }
    .temperature-box {
      border-color: #059e8a;
    }
    .humidity-box {
      border-color: #00add6;
    }
    .value {
      font-size: 2.5rem;
      font-weight: bold;
    }
    .units {
      font-size: 1.2rem;
      vertical-align: top;
    }
    .dht-labels {
      font-size: 1.5rem;
    }
  </style>
</head>
<body>
  <h2>ESP32 Weather Station</h2>
  
  <!-- Temperature Box -->
  <div class="box temperature-box">
    <div class="box-header">
      <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
      <div class="dht-labels">Temperature</div>
    </div>
    <div>
      <span id="temperature" class="value">%TEMPERATURE%</span>
      <sup class="units">&deg;C</sup>
    </div>
  </div>
  
  <!-- Humidity Box -->
  <div class="box humidity-box">
    <div class="box-header">
      <i class="fas fa-tint" style="color:#00add6;"></i>
      <div class="dht-labels">Humidity</div>
    </div>
    <div>
      <span id="humidity" class="value">%HUMIDITY%</span>
      <sup class="units">&percnt;</sup>
    </div>
  </div>
</body>
<script>
  // Update temperature every 2 seconds
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
  }, 2000);

  // Update humidity every 2 seconds
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("humidity").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/humidity", true);
    xhttp.send();
  }, 2000);
</script>
</html>)rawliteral";
/*===================HTML template for the web server==================*/

/*=====Process the placeholders in the HTML with actual data===========*/
String processor(const String &var) {
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  }
  else if (var == "HUMIDITY") {
    return readDHTHumidity();
  }
  return String();
}

void setup() {
  Serial.begin(9600);

  // Initialize DHT sensor
  dht11.begin();

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");

  // Initialize RGB LED pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    if (millis() - lastWiFiCheck >= WiFiCheckInterval) {
      Serial.println("Connecting to WiFi...");
      lastWiFiCheck = millis();
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Display IP address when Wi-Fi is connected
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    // Display error message if Wi-Fi connection fails
    Serial.println("WiFi connection failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  // Start the web server
  server.begin();
}

void loop() {
  unsigned long currentMillis = millis();  

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  

    // Read data from DHT sensor
    String humidityStr = readDHTHumidity();
    String temperatureStr = readDHTTemperature();

    // Display data on LCD
    lcd.clear();
    if (humidityStr == "--" || temperatureStr == "--") {
      lcd.setCursor(0, 0);
      lcd.print("Failed to read");
      lcd.setCursor(0, 1);
      lcd.print("DHT sensor!");
      Serial.println("Failed to read from DHT sensor");
    } else {
      float temperature = temperatureStr.toFloat();
      if (temperature < 25) {
        setColor(HIGH, HIGH, LOW); 
        lcd.setCursor(2, 0);
        lcd.print("Temp too cold");
      } else if (temperature > 30) {
        setColor(LOW, HIGH, HIGH); 
        lcd.setCursor(2, 0);
        lcd.print("Temp too hot");
      } else {
        setColor(HIGH, LOW, HIGH); 
        lcd.setCursor(2, 0);
        lcd.print("Temp normal");
      }
    }
  }

  // Check Wi-Fi connection every 3 seconds
  if (currentMillis - lastWiFiCheck >= WiFiCheckInterval) {
    lastWiFiCheck = currentMillis;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnecting to WiFi...");
      WiFi.reconnect();
    } else {
      Serial.println(WiFi.localIP());
    }
  }
}
