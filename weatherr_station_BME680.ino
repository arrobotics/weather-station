#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define RAIN_SENSOR_PIN A0

const char* ssid = "KRISHI_SARTHI";
const char* pass = "12345678";

Adafruit_BME680 bme; // I2C
ESP8266WebServer server(80);

// Sensor values
float temperature = 0;
float humidity = 0;
float pressure = 0;
float gas = 0;
int rainValue = 0;
String rainStatus = "Unknown";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(RAIN_SENSOR_PIN, INPUT);

  if (!bme.begin()) {
    Serial.println("Could not find BME680 sensor!");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  if (!bme.performReading()) {
    Serial.println("Failed to read from BME680!");
    return;
  }

  temperature = bme.temperature;
  humidity = bme.humidity;
  pressure = bme.pressure / 100.0;
  gas = bme.gas_resistance / 1000.0;

  rainValue = analogRead(RAIN_SENSOR_PIN);
  rainStatus = (rainValue < 500) ? "Rain Detected" : "Dry";

  Serial.print("Rain Sensor Analog Value: ");
  Serial.print(rainValue);
  Serial.print(" | Status: ");
  Serial.println(rainStatus);

  delay(2000);
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(temperature, humidity, pressure, gas, rainStatus));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float t, float h, float p, float g, String rain) {
  String ptr = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>BME680 Sensor Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css">
  <style>
    body {
      margin: 0;
      padding: 0;
      font-family: 'Segoe UI', sans-serif;
      background: linear-gradient(to bottom right, #d0eaff, #ffffff);
      color: #333;
    }
    header {
      background: rgba(255, 255, 255, 0.2);
      backdrop-filter: blur(10px);
      box-shadow: 0 8px 32px rgba(31, 38, 135, 0.2);
      text-align: center;
      padding: 30px 10px;
      border-radius: 0 0 30px 30px;
      margin-bottom: 20px;
    }
    header h1 {
      font-size: 2.2rem;
      font-weight: bold;
      color: #006d77;
    }
    .container {
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 10px;
    }
    .card {
      background: #ffffffc2;
      border-radius: 20px;
      box-shadow: 0 8px 20px rgba(0,0,0,0.1);
      margin: 15px;
      padding: 20px;
      width: 90%;
      max-width: 400px;
      text-align: center;
    }
    .icon {
      font-size: 2.5rem;
      margin-bottom: 10px;
    }
    .value {
      font-size: 2rem;
      font-weight: bold;
    }
    .units {
      font-size: 1rem;
      color: #555;
    }
    footer {
      text-align: center;
      font-size: 0.9rem;
      padding: 10px;
      color: #777;
    }
    @media (max-width: 600px) {
      header h1 {
        font-size: 1.7rem;
      }
      .value {
        font-size: 1.6rem;
      }
    }
  </style>
  <script>
    setInterval(function(){ location.reload(); }, 5000);
  </script>
</head>
<body>
  <header><h1>Weather Station  Dashboard</h1></header>
  <div class="container">
    <div class="card">
      <div class="icon" style="color:#f7347a;"><i class="fas fa-temperature-high"></i></div>
      <div class="value">)rawliteral" + String(t, 1) + R"rawliteral(&deg;C</div>
      <div class="units">Temperature</div>
    </div>

    <div class="card">
      <div class="icon" style="color:#0077be;"><i class="fas fa-tint"></i></div>
      <div class="value">)rawliteral" + String(h, 1) + R"rawliteral( %</div>
      <div class="units">Humidity</div>
    </div>

    <div class="card">
      <div class="icon" style="color:#a70000;"><i class="fas fa-tachometer-alt"></i></div>
      <div class="value">)rawliteral" + String(p, 1) + R"rawliteral( hPa</div>
      <div class="units">Pressure</div>
    </div>

    <div class="card">
      <div class="icon" style="color:#cc3300;"><i class="fas fa-burn"></i></div>
      <div class="value">)rawliteral" + String(g, 1) + R"rawliteral( KΩ</div>
      <div class="units">Gas Resistance</div>
    </div>

    <div class="card">
      <div class="icon" style="color:#3b82f6;"><i class="fas fa-cloud-rain"></i></div>
      <div class="value">)rawliteral" + rain + R"rawliteral(</div>
      <div class="units">Rain Status</div>
    </div>
  </div>
  <footer>AR RoboTics © 2025</footer>
</body>
</html>
)rawliteral";
  return ptr;
}
