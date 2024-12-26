#define BLYNK_TEMPLATE_ID "TMPL60CK-yn9T"
#define BLYNK_TEMPLATE_NAME "Smart Farm"
#define BLYNK_AUTH_TOKEN "T-2FcGyM91GmE8JfRPO9wh5j-sMx3_tt"
#define BLYNK_PRINT Serial

#define SOIL_MOISTURE_PIN A0  // Chân cảm biến độ ẩm đất (GPIOA0)
#define PUMP_PIN 5            // Chân máy bơm (GPIO5)
#define HUMIDIFIER_PIN 4      // Chân máy tạo ẩm (GPIO4)
#define HEATER_PIN 0          // Chân máy sưởi (GPIO0)
#define DHTPIN 2      // Chân DHT11 (GPIO2 trên ESP8266)
#define DHTTYPE DHT11 // Loại cảm biến DHT

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
//#include <DHT11.h>

char auth[] = BLYNK_AUTH_TOKEN;  // Mã Blynk của bạn
char ssid[] = "Redmi 13";     // Tên mạng Wi-Fi
char pass[] = "12345678";                // Mật khẩu Wi-Fi (trống nếu không có mật khẩu)

//DHT11 dht11(DHTPIN);
DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// Khai báo các biến lưu trữ thông tin
float temp;            // Nhiệt độ hiện tại
float humidity;        // Độ ẩm hiện tại
int soilMoisture;      // Độ ẩm đất hiện tại

int pumpStatus;        // Trạng thái máy bơm
int humidifierStatus;  // Trạng thái máy tạo ẩm
int heaterStatus;      // Trạng thái máy sưởi

int tempSetting;       // Nhiệt độ cài đặt từ Blynk
int humiditySetting;   // Độ ẩm cài đặt từ Blynk
int soilMoistureSetting; // Độ ẩm đất cài đặt từ Blynk

// Hàm xử lý thông số
void processParameters() {
    if (temp < tempSetting) {
        heaterStatus = 1;   // Bật máy sưởi nếu nhiệt độ thấp hơn cài đặt
    } else {
        heaterStatus = 0;   // Tắt máy sưởi
    }

    if (humidity < humiditySetting) {
        humidifierStatus = 1;   // Bật máy tạo ẩm nếu độ ẩm thấp hơn cài đặt
    } else {
        humidifierStatus = 0;   // Tắt máy tạo ẩm
    }

    if (soilMoisture < soilMoistureSetting) {
        pumpStatus = 1;   // Bật máy bơm nếu độ ẩm đất thấp hơn cài đặt
    } else {
        pumpStatus = 0;   // Tắt máy bơm
    }
}

// Hàm gửi dữ liệu lên Blynk
void sendDataToBlynk() {
    temp = dht.readTemperature();        // Đọc nhiệt độ từ DHT11
    humidity = dht.readHumidity();       // Đọc độ ẩm từ DHT11
    //int result  = dht11.readTemperatureHumidity(temp, humidity);
    // if (result == 0) {
    // }else{
    //   Serial.println("Cant read data on DHT11");
    // };
    int humi_ground_raw = 0;
    for(int i=0;i<=9;i++){
    humi_ground_raw+=analogRead(SOIL_MOISTURE_PIN);
    }
    humi_ground_raw=humi_ground_raw/10;
    int soilMoisture = 100 - map(humi_ground_raw, 350, 1023, 0, 100);
    Blynk.virtualWrite(V0, temp);                // Gửi nhiệt độ lên V0
    Blynk.virtualWrite(V1, humidity);            // Gửi độ ẩm lên V1
    Blynk.virtualWrite(V2, soilMoisture);        // Gửi độ ẩm đất lên V2

    Blynk.virtualWrite(V6, pumpStatus);          // Gửi trạng thái máy bơm lên V6
    Blynk.virtualWrite(V7, humidifierStatus);    // Gửi trạng thái máy tạo ẩm lên V7
    Blynk.virtualWrite(V8, heaterStatus);        // Gửi trạng thái máy sưởi lên V8
}

// Các hàm nhận giá trị cài đặt từ Blynk
BLYNK_WRITE(V3) { tempSetting = param.asInt(); }       // Cài đặt nhiệt độ từ V3
BLYNK_WRITE(V4) { humiditySetting = param.asInt(); }   // Cài đặt độ ẩm từ V4
BLYNK_WRITE(V5) { soilMoistureSetting = param.asInt(); } // Cài đặt độ ẩm đất từ V5

void setup() {
    Serial.begin(9600);
    Blynk.begin(auth, ssid, pass);
    dht.begin();

    pinMode(PUMP_PIN, OUTPUT);
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    pinMode(HEATER_PIN, OUTPUT);

    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
    digitalWrite(HEATER_PIN, LOW);

    timer.setInterval(5000L, sendDataToBlynk);  // Gửi dữ liệu mỗi 5 giây
}

void loop() {
    Blynk.run();
    timer.run();
    processParameters();   // Cập nhật trạng thái thiết bị
}
