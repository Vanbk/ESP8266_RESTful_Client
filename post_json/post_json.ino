#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

int deviceId = 199;
int timeStamp = 0;
float temperature = 14;
float humidity = 50;
float pm25 = 50;
float pm10 = 60;
int act1State = 0;
int act2State = 0;

int increase_pm10 = 1;
int increase_pm25 = 1; 

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);                            //Serial connection
  WiFi.begin("NCL_air", "ncl@ee614soc!" );   //WiFi connection
 
  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
 
    delay(500);
    Serial.println("Waiting for connection");
  }
}
 
void loop() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
 
     // mock data
    temperature = temperature + random(1,10)*0.01;
    if (temperature > 23.4) temperature = 23.4;
    humidity = humidity + random(1, 10)*0.01;
    if (humidity > 70.3) humidity = 70.3;

    if (pm10 > 150) increase_pm10 = -1;
    if (pm10 <= 60) increase_pm10 = 1;
    pm10 = pm10 + random(1, 10)*0.01*increase_pm10;
    
    if (pm25 > 100) increase_pm25 = -1;
    if (pm25 <= 50) increase_pm25 = 1;
    pm25 = pm25 + random(1, 10)*0.01*increase_pm25;
    
    // Prepare JSON series to sending 
    StaticJsonBuffer<300> JSONbuffer;   //Declaring static JSON buffer
    JsonObject& JSONencoder = JSONbuffer.createObject(); 
 
    JSONencoder["deviceId"] = deviceId;
    JSONencoder["timeStamp"] = timeStamp;
    JSONencoder["temperature"] = temperature;
    JSONencoder["humidity"] = humidity;
    JSONencoder["pm25"] = pm25;
    JSONencoder["pm10"] = pm10;
    JSONencoder["act1State"] = act1State;
    JSONencoder["act2State"] = act2State;
    
    /*
    JsonArray& values = JSONencoder.createNestedArray("values"); //JSON array
    values.add(20); //Add value to array
    values.add(21); //Add value to array
    values.add(23); //Add value to array
 
    JsonArray& timestamps = JSONencoder.createNestedArray("timestamps"); //JSON array
    timestamps.add("10:10"); //Add value to array
    timestamps.add("10:20"); //Add value to array
    timestamps.add("10:30"); //Add value to array
    */
    char JSONmessageBuffer[300];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
 
    HTTPClient http;    //Declare object of class HTTPClient
 
    http.begin("http://10.0.1.31:9000/data");      //Specify request destination
    http.addHeader("Content-Type", "application/json");  //Specify content-type header
 
    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
    String payload = http.getString();                                        //Get the response payload
    
    Serial.println(httpCode);   //Print HTTP return code
    Serial.println(payload);    //Print request response payload

    // parse the responde to get the relay status
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    // Test if parsing succeeds.
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    const char* sensor = root["message"];
    act1State = root["act1State"];
    act2State = root["act2State"];
    
    digitalWrite(LED_BUILTIN, !act1State);
    
    Serial.println (sensor);
    Serial.println(act1State);
    Serial.println(act2State);
    http.end();  //Close connection
 
  } else {
 
    Serial.println("Error in WiFi connection");
 
  }
  delay(2000);  //Send a request every 2 seconds
}
