#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 1     //将DHT11传感器DATA引脚连接到ESP32C3的GPIO4引脚
#define DHTTYPE DHT11   // DHT11传感器类型

DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char *ssid = "myssid"; // Enter your WiFi name
const char *password = "mypassword";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.x.x.x";
const char *topic = "Topic/TempHumSub";   //消息接受，设定值输入
const char *topic1 = "Topic/TempHumPub";    //消息发布主题
const char *mqtt_username = "mqtt_username";
const char *mqtt_password = "mqtt_password";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  // Set software serial baud to 115200;
  Serial.begin(115200);   
  // connecting to a WiFi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) 
  {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public mqtt broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("Public emqx mqtt broker connected");
    } 
    else 
    {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // publish and subscribe
  //client.publish(topic1, "Hi EMQX I'm ESP32 ^^");
  //client.subscribe(topic);
}

void callback(char *topic, byte *payload, unsigned int length) 
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) 
  {
    Serial.print((char) payload[i]);
  }
}

void loop() 
{
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  delay(200);
  neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
  delay(1800);
  
  float temperature = dht.readTemperature(); // 读取温度
  float humidity = dht.readHumidity(); // 读取湿度
  
  // 生成要发送的 JSON 数据
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Temp"] = temperature;
  jsonDoc["Hum"] = humidity;
  String jsonData;
  serializeJson(jsonDoc, jsonData);
  client.publish(topic1,jsonData.c_str());
  client.loop();
}
