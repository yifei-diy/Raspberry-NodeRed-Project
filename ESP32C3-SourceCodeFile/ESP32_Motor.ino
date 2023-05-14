#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// WiFi
const char *ssid = "myssid"; // Enter your WiFi name
const char *password = "mypassword";  // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "192.x.x.x";
const char *topic = "Topic/MotorSub";  //消息接受，设定值输入
const char *topic1 = "Topic/CurrentVoltagePub";    //消息发布主题
const char *mqtt_username = "mqtt_username";
const char *mqtt_password = "mqtt_password";
const int mqtt_port = 1883;

int MotorSet;
int Relay_Pin=1;


WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  // Set software serial baud to 115200;
  Serial.begin(115200);   
//set the resolution to 12 bits (0-4096)
  analogReadResolution(12);
  pinMode(Relay_Pin, OUTPUT);
  
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
    MotorSet = setAlinkMsgObj["MotorSet"]; 
    //Serial.println(SETLUM);
}

void loop() 
{
   // read the analog / millivolts value for pin 2:
  int analogCurrent = analogRead(0);
   // read the analog / millivolts value for pin 2:
  int analogVoltage = analogRead(2);
  
  float Current_Value=((analogCurrent*3.2)/4096);
  float Voltage_Value=((analogVoltage*3.2)/4096)/0.095;
  
  if(MotorSet==1)
  {
    digitalWrite(Relay_Pin, HIGH);   // 打开继电器
    //Serial.printf("Start:ON\n");
  }
  else if(MotorSet==0)
  {
    digitalWrite(Relay_Pin, LOW);   // 关闭继电器
    //Serial.printf("Start:OFF\n");
  }
  
   // print out the values you read:
  //Serial.printf("Current_Value = %f\n",Current_Value);
  //Serial.printf("Voltage_Value = %f\n",Voltage_Value);
  //Serial.printf("MotorSet = %d\n",MotorSet);
  //Serial.println(SETLUM);
  // 生成要发送的 JSON 数据
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["Current"] = Current_Value;
  jsonDoc["Voltage"] = Voltage_Value;
  String jsonData;
  serializeJson(jsonDoc, jsonData);
  
  neopixelWrite(RGB_BUILTIN,RGB_BRIGHTNESS,0,0); // Red
  delay(50);
  neopixelWrite(RGB_BUILTIN,0,0,0); // Off / black
  delay(80);
  client.publish(topic1,jsonData.c_str());
  client.loop();
}
