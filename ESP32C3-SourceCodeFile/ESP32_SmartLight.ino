#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi
const char *ssid = "myssid"; // Enter your WiFi name
const char *password = "mypassword";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.x.x.x";
const char *topic = "Topic/LumSub";  //消息接受，设定值输入
const char *topic1 = "Topic/LumPub";    //消息发布主题
const char *mqtt_username = "mqtt_username";
const char *mqtt_password = "mqtt_password";
const int mqtt_port = 1883;

int SETLUM;

int freq = 2000;    // 频率
int channel = 0;    // 通道
int resolution = 8;   // 分辨率

const int led = 18;

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  // Set software serial baud to 115200;
  Serial.begin(115200);   
//set the resolution to 12 bits (0-4096)
  analogReadResolution(12);

  ledcSetup(channel, freq, resolution); // 设置通道
  ledcAttachPin(led, channel);  // 将通道与对应的引脚连接
  
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
    neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black 
  client.subscribe(topic);
 
}
// Define the callback function
void callback(char* topic, byte* payload, unsigned int length)
{

    //Serial.println(topic);
    //Serial.println((char *)payload);

    //接下来是收到的json字符串的解析
    DynamicJsonDocument doc(100);
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.println("parse json failed");
      return;
    }
    JsonObject setAlinkMsgObj = doc.as<JsonObject>();
    serializeJsonPretty(setAlinkMsgObj, Serial);
    Serial.println();

    //这里是一个点灯小逻辑
    SETLUM = setAlinkMsgObj["SetLum"]; 
    //Serial.println(SETLUM);
}

void loop() 
{
  int LED_Control=int(0.255*SETLUM);
  ledcWrite(channel, LED_Control);  // 输出PWM
   
   // read the analog / millivolts value for pin 2:
  int analogValue = analogRead(2);
  
  float LumanalogValue=(analogValue*3.3)/4096;
  float Lum_Val=303.03*LumanalogValue;
  
   // print out the values you read:
  Serial.printf("ADC analog value = %f\n",Lum_Val);
  //Serial.println(SETLUM);
  // 生成要发送的 JSON 数据
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Lum"] = Lum_Val;
  String jsonData;
  serializeJson(jsonDoc, jsonData);
  
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  delay(50);
  neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
  delay(80);
  client.publish(topic1,jsonData.c_str());
  client.loop();
}
