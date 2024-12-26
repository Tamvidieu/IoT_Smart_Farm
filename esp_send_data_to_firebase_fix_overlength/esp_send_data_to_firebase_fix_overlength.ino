#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>

#define FIREBASE_HOST "https://smart-farm-bbb89-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "Gs7paSYqbHjSLQ50DKS1K8XtKMtlpxm3u3kywMlw"
#define WIFI_SSID "iot ptit"
#define WIFI_PASSWORD "1234567890"

// Chân kết nối thiết bị
#define HEATER_PIN 14      // GPIO2 (D4) 
#define HUMIDIFIER_PIN 12  // GPIO12 (D6)
#define PUMP_PIN 13        // GPIO13 (D7)

// Chân cảm biến DHT và loại cảm biến
#define DHT_PIN 2            // GPIO14 (D5)
#define DHTTYPE DHT11

// Chân cảm biến độ ẩm đất
#define HUMI_GROUND_PIN A0

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Firebase
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

// DHT sensor
DHT dht(DHT_PIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    dht.begin();

    pinMode(HEATER_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    digitalWrite(HEATER_PIN, HIGH);
    digitalWrite(HUMIDIFIER_PIN, HIGH);
    digitalWrite(PUMP_PIN, HIGH);

        // Khởi tạo LCD
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("Connecting wifi");

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    lcd.setCursor(0,1);
    lcd.print("Connected wifi");
    // Cấu hình Firebase
    config.host = FIREBASE_HOST;
    config.signer.tokens.legacy_token = FIREBASE_AUTH;

    Firebase.begin(&config, &auth);

    // Đồng bộ thời gian
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    while (!time(nullptr)) {
        Serial.print(".");
        delay(500);
    }
}

int getFirebaseValue(const String &key) {
    delay(50);
    if (Firebase.getString(firebaseData, key)) {
        String valueStr = firebaseData.stringData();
        return valueStr.toInt();
    } else {
        Serial.println("Failed to get " + key + ": " + firebaseData.errorReason());
        return -1;
    }
}

void sendData(const String &key, int value) {
    if (Firebase.setInt(firebaseData, key, value)) {
    } else {
        Serial.println("Failed to send " + key + ": " + firebaseData.errorReason());
    }
    delay(50);
}

int readSoilMoisture() {
    int humi_ground_raw = 0;
    for (int i = 0; i < 10; i++) {
        humi_ground_raw += analogRead(HUMI_GROUND_PIN);
        delay(10);
    }
    humi_ground_raw /= 10;
    int humi_ground = 100 - map(humi_ground_raw, 350, 1023, 0, 100);
    humi_ground = constrain(humi_ground, 0, 100); // Đảm bảo giá trị trong khoảng 0-100%
    return humi_ground;
}

void loop() {
    // Đọc giá trị cảm biến
    int humidity = dht.readHumidity();
    int temperature = dht.readTemperature();
    if(humidity > 100 || temperature > 100){
      humidity=-99;
      temperature=-99;
    }
    int soilMoisture = readSoilMoisture();

    // Lấy ngưỡng từ Firebase
    int humidityTrigger = getFirebaseValue("/realtime_data/trigger/humidity");
    int soilMoistureTrigger = getFirebaseValue("/realtime_data/trigger/soil_moisture");
    int temperatureTrigger = getFirebaseValue("/realtime_data/trigger/temperature");
    if (humidityTrigger == -1 || soilMoistureTrigger == -1 || temperatureTrigger == -1) {
        Serial.println("Cant read trigger on Firebase!");
        return;
    }

    // Điều khiển thiết bị
    bool stateHumidifier = humidity < humidityTrigger;
    digitalWrite(HUMIDIFIER_PIN, !stateHumidifier);
    bool statePump = soilMoisture < soilMoistureTrigger;
    digitalWrite(PUMP_PIN, !statePump);
    bool stateHeater = temperature < temperatureTrigger;
    digitalWrite(HEATER_PIN, !stateHeater);

    // Gửi dữ liệu cảm biến lên Firebase
    if(humidity < 0 || temperature < 0){
      sendData("/realtime_data/sensors/temperature", temperature);
      sendData("/realtime_data/sensors/humidity", humidity);
    }
    sendData("/realtime_data/sensors/soil_moisture", soilMoisture);
    sendData("/realtime_data/peripheral/humidifier", stateHumidifier ? 1 : 0);
    sendData("/realtime_data/peripheral/pump", statePump ? 1 : 0);
    sendData("/realtime_data/peripheral/heater", stateHeater ? 1 : 0);

    // Gửi dữ liệu lịch sử lên Firebase
    time_t now = time(nullptr);
    String historyPath = "/history_data/" + String(now) + "/";
    if(humidity < 0 || temperature < 0){
      sendData(historyPath + "sensors/humidity", humidity);
      sendData(historyPath + "sensors/temperature", temperature);
    }
    sendData(historyPath + "sensors/soil_moisture", soilMoisture);
    sendData(historyPath + "peripheral/humidifier", stateHumidifier ? 1 : 0);
    sendData(historyPath + "peripheral/pump", statePump ? 1 : 0);
    sendData(historyPath + "peripheral/heater", stateHeater ? 1 : 0);
    
    // Hiển thị thông tin trên LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("S:" + String(soilMoisture) + "% T:" + String(temperature) + "C H:" + String(humidity) + "%");
    lcd.setCursor(0, 1);
    String states = "P:" + String(statePump ? "ON" : "OFF") + " H:" + String(stateHeater ? "ON" : "OFF") + " M:" + String(stateHumidifier ? "ON" : "OFF");
    lcd.print(states);
    
    // Log thông tin ra Serial
    Serial.printf(
        "Process successful !!! \nTemperature: %d°C, Humidity: %d%%, Soil Moisture: %d%% \n Heater: %s, Humidifier: %s, Pump: %s\n TRIGGER:Temperature: %d°C, Humidity: %d%%, Soil Moisture: %d%%\n\n",
        temperature, humidity, soilMoisture,
        stateHeater ? "ON " : "OFF",
        stateHumidifier ? "ON " : "OFF",
        statePump ? "ON " : "OFF",
        temperatureTrigger, humidityTrigger, soilMoistureTrigger
    );
}
