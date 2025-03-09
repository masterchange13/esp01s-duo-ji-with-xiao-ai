#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>  // 使用 ESP8266 兼容的 Servo 库

// WiFi 配置
const char* ssid = "MiFi-5A26";     
const char* password = "1234567890";  

// MQTT 配置
const char* mqtt_server = "bemfa.com";  
uint16_t mqtt_server_port = 9501;       
#define ID_MQTT  "2a2c6fe8997b489da138b231a8f887e7"  
const char* topic = "light002";  

WiFiClient espClient;
PubSubClient client(espClient);

// 舵机配置
#define SERVO_PIN 2  // ESP-01S 可用的 GPIO2
Servo myservo;  

const int STOP_ANGLE = 90;  // 舵机停止角度（不同品牌可能需微调）
const int ROTATE_TIME = 500;  // 旋转持续时间（毫秒）

// 连接 WiFi
void setup_wifi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 20) {  // 最多重试 10 秒
        delay(500);
        Serial.print(".");
        retries++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
    } else {
        Serial.println("\nWiFi连接失败，请检查SSID和密码");
    }
}

// MQTT 消息回调函数
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.print("收到 MQTT 消息: ");
    Serial.println(message);

    if (message == "on") {
        myservo.write(180);  // 顺时针旋转
        delay(ROTATE_TIME);
        myservo.write(STOP_ANGLE);  // 停止
        Serial.println("舵机顺时针旋转");
    } else if (message == "off") {
        myservo.write(0);    // 逆时针旋转
        delay(ROTATE_TIME);
        myservo.write(STOP_ANGLE);  // 停止
        Serial.println("舵机逆时针旋转");
    } else if (message == "stop") {
        myservo.write(STOP_ANGLE);  // 停止
        Serial.println("舵机停止");
    }
}

// MQTT 连接
void MQTT_reconnect() {
    while (!client.connected()) {
        Serial.print("尝试连接 MQTT...");
        if (client.connect(ID_MQTT)) {
            Serial.println("MQTT 连接成功！");
            client.subscribe(topic);
            Serial.println("订阅主题：" + String(topic));
        } else {
            Serial.print("连接失败，错误代码 = ");
            Serial.print(client.state());
            Serial.println("，5 秒后重试...");
            delay(5000);
        }
    }
}

// 初始化
void setup() {
    Serial.begin(115200);
    myservo.attach(SERVO_PIN);  // 连接舵机
    myservo.write(STOP_ANGLE);  // 初始状态设为停止

    setup_wifi();  
    client.setServer(mqtt_server, mqtt_server_port);
    client.setCallback(callback);
}

// 主循环
void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        setup_wifi();  // WiFi 断线自动重连
    }

    if (!client.connected()) {
        MQTT_reconnect();  
    }
    client.loop();
}
