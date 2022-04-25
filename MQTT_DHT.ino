/**********************************************************************
项目名称/Project          : 零基础入门学用物联网
程序名称/Program name     : pub_and_sub
团队/Team                : 太极创客团队 / Taichi-Maker (www.taichi-maker.com)
作者/Author              : CYNO朔
日期/Date（YYYYMMDD）     : 20200719
程序目的/Purpose          : 
利用PubSubClient同时订阅和发布信息。
定时向主题发布D3按键的状态
通过订阅主题收到信息是数字1则打开LED，否则关闭LED。
-----------------------------------------------------------------------
本示例程序为太极创客团队制作的《零基础入门学用物联网》中示例程序。
该教程为对物联网开发感兴趣的朋友所设计和制作。如需了解更多该教程的信息，请参考以下网页：
http://www.taichi-maker.com/homepage/esp8266-nodemcu-iot/iot-c/esp8266-nodemcu-web-client/http-request/
***********************************************************************/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include "DHT.h"
//#include <ArduinoJson.h> 
#define DHTPIN 13     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11 
// 设置wifi接入信息(请根据您的WiFi信息进行修改)
const char* ssid = "Dell";
const char* password = "1136759016";
const char* mqttServer = "3uq976k.mqtt.iot.gz.baidubce.com";
// 如以上MQTT服务器无法正常连接，请前往以下页面寻找解决方案
// http://www.taichi-maker.com/public-mqtt-broker/
 
Ticker ticker;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
 
int count;    // Ticker计数用变量
const int subQoS = 1;     // 客户端订阅主题时使用的QoS级别（截止2020-10-07，仅支持QoS = 1，不支持QoS = 2）
const bool cleanSession = false; // 清除会话（如QoS>0必须要设为false）
const char* mqttUserName = "3uq976k/mqttclient"; // 服务端连接用户名
const char* mqttPassword = "n46GT4yJFS10Fm9d"; // 服务端连接密码
DHT dht(DHTPIN, DHTTYPE); 
String H_T ;
//StaticJsonDocument<64> doc;

  
void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);     // 设置板上LED引脚为输出模式
  digitalWrite(LED_BUILTIN, HIGH);  // 启动后关闭板上LED 
  pinMode(4, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  // Ticker定时对象
  ticker.attach(1, tickerCount);
  
  //设置ESP8266工作模式为无线终端模式
  WiFi.mode(WIFI_STA);
  
  // 连接WiFi
  connectWifi();
  
  // 设置MQTT服务器和端口号
  mqttClient.setServer(mqttServer, 1883);
  mqttClient.setCallback(receiveCallback);
 
  // 连接MQTT服务器
  connectMQTTServer();
  dht.begin();
//  doc["temp"] = 100;
//  doc["humi"] = 100;
//  serializeJson(doc, H_T);
}
 
void loop() {
  if (mqttClient.connected()) { // 如果开发板成功连接服务器
    // 每隔3秒钟发布一次信息
    if (count >= 3){
      pubMQTTmsg();
      count = 0;
    }    
    // 保持心跳
    mqttClient.loop();
  } else {                  // 如果开发板未能成功连接服务器
    connectMQTTServer();    // 则尝试连接服务器
  }
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
//  doc["temp"] = t;
//  doc["humi"] = h;
    String T=String(t);
    String H=String(h);
    H_T = "{\"temp\":\"" + T + "\",\"humi\":\"" + H + "\"}\n";
//  Serial.println(t);
//  Serial.println(h);
//  serializeJson(doc, H_T);
}
 
void tickerCount(){
  count++;
}
 
void connectMQTTServer(){
  // 根据ESP8266的MAC地址生成客户端ID（避免与其它ESP8266的客户端ID重名）
  String clientId = "esp8266-DHT";
 
  // 连接MQTT服务器
  if (mqttClient.connect(clientId.c_str(), mqttUserName, 
                         mqttPassword)) { 
    Serial.println("MQTT Server Connected.");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.println("ClientId:");
    Serial.println(clientId);
    subscribeTopic(); // 订阅指定主题
  } else {
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient.state());
    delay(3000);
  }   
}
 
// 发布信息
void pubMQTTmsg(){
  // 建立发布主题。主题名称以Taichi-Maker-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP8266客户端名称各不相同，
  String topicString = "TOPIC3";
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());
  String messageString = H_T;
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  if(mqttClient.publish(publishTopic, publishMsg)){
    Serial.println("Publish Topic:");Serial.println(publishTopic);
    Serial.println("Publish message:");Serial.println(publishMsg);    
  } else {
    Serial.println("Message Publish Failed."); 
  }
}
 
// 订阅指定主题
void subscribeTopic(){
 
  // 建立订阅主题。主题名称以Taichi-Maker-Sub为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  String topicString = "TOPIC1";
  char subTopic[topicString.length() + 1];  
  strcpy(subTopic, topicString.c_str());
  
  // 通过串口监视器输出是否成功订阅主题以及订阅的主题名称
  if(mqttClient.subscribe(subTopic)){
    Serial.println("Subscribe Topic:");
    Serial.println(subTopic);
  } else {
    Serial.print("Subscribe Fail...");
  }  
}
 
// 收到信息后的回调函数
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
 
  if ((char)payload[0] == '1') {     // 如果收到的信息以“1”为开始
    digitalWrite(BUILTIN_LED, LOW);  // 则点亮LED。
    digitalWrite(5, LOW);
  } else {                           
    digitalWrite(BUILTIN_LED, HIGH); // 否则熄灭LED。
    digitalWrite(5, HIGH); // 否则熄灭LED
  }
}
 
// ESP8266连接wifi
void connectWifi(){
 
  WiFi.begin(ssid, password);
 
  //等待WiFi连接,成功连接后输出成功信息
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected!");  
  Serial.println(""); 
}
