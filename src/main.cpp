#include <WiFi.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include "ESPAsyncWebServer.h"

#define DHTTYPE DHT11                     // DHT 11
#define DHT11_PIN 23                      // Pin cảm biến DHT11 kết nối với ESP32
#define RED_PIN   15                      // Pin Red of RGB
#define GREEN_PIN 16                      // Pin Green of RGB
#define BLUE_PIN  17                      // Pin Blue of RGB

const char* ssid = "Nga.Ntt";            // SSID Wi-Fi
const char* password = "08032004";       // Mật khẩu Wi-Fi

LiquidCrystal_I2C lcd(0x27, 16, 2);      // Địa chỉ I2C 0x27, LCD 16x2
DHT dht11(DHT11_PIN, DHT11);             // Khởi tạo đối tượng DHT

AsyncWebServer server(80);               // Khởi tạo server trên cổng 80

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

// Hàm đọc nhiệt độ
String readDHTTemperature() {
  float t = dht11.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(t);
  }
}

// Hàm đọc độ ẩm
String readDHTHumidity() {
  float h = dht11.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    return String(h);
  }
}

// Trang HTML cho Webserver
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
  
  <!-- Khung Nhiệt Độ -->
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
  
  <!-- Khung Độ Ẩm -->
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
  // Cập nhật nhiệt độ mỗi 10 giây
  setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("temperature").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "/temperature", true);
    xhttp.send();
  }, 10000);

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

// Thay thế các placeholder trong HTML với giá trị nhiệt độ và độ ẩm
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

  // Khởi tạo cảm biến DHT
  dht11.begin();

  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi...");

  // Initialize RGB LED
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  // Kết nối Wi-Fi
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 10) {
    delay(3000);
    Serial.println("Connecting to WiFi...");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    // Hiển thị địa chỉ IP khi kết nối thành công
    Serial.println(WiFi.localIP());
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Connected");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    // Thông báo lỗi nếu không thể kết nối
    Serial.println("WiFi connection failed");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed");
  }

  // Cấu hình server để trả về HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Cấu hình server để trả về nhiệt độ
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });

  // Cấu hình server để trả về độ ẩm
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  // Bắt đầu chạy server
  server.begin();
}

void loop() {
  // Đọc dữ liệu từ cảm biến DHT
  String humidityStr = readDHTHumidity();
  String temperatureStr = readDHTTemperature();

  // Hiển thị trạng thái trên LCD
  lcd.clear();
  if (humidityStr == "--" || temperatureStr == "--") {
    lcd.setCursor(0, 0);
    lcd.print("Failed to read");
    lcd.setCursor(0, 1);
    lcd.print("DHT sensor!");
    Serial.println("Failed to read from DHT sensor");
  } else {
    // lcd.setCursor(0, 0);
    // lcd.print("Temp: ");
    // lcd.print(temperatureStr);
    // lcd.print("C");
    // lcd.setCursor(0, 1);
    // lcd.print("Humi: ");
    // lcd.print(humidityStr);
    // lcd.print("%");

    float temperature = temperatureStr.toFloat();
    if (temperature < 25) {
      setColor(HIGH, HIGH, LOW); // (blue)
      lcd.setCursor(2, 0);
      lcd.print("Temp too cold");
    } else if (temperature > 30) {
      flashColor(LOW, HIGH, HIGH, 200);
      setColor(LOW, HIGH, HIGH); // (red)
      lcd.setCursor(2, 0);
      lcd.print("Temp too hot");
    } else {
      setColor(HIGH, LOW, HIGH); // (green)
      lcd.setCursor(2, 0);
      lcd.print("Temp normal");
    }

  }

  // Delay 2 giây để cập nhật lại thông tin trên LCD
  delay(2000);
}
