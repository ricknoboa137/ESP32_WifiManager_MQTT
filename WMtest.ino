#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <PubSubClient.h>



char *strings[3];
float velocity_values[3]={0,0,0};
char *ptr = NULL;
uint32_t x=0;
char mqtt_server[40]= "192.168.0.110";
char mqtt_port[6] = "1883";

WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiClient espClient;
PubSubClient client(espClient);


/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    //WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    Serial.begin(115200);
    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();
    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    wm.addParameter(&custom_mqtt_server);
    wm.addParameter(&custom_mqtt_port);
    wm.setSaveParamsCallback(saveParamCallback);
    wm.setClass("invert"); // use darkmode

    client.setServer(mqtt_server, atoi(mqtt_port));
    client.setCallback(callback);


    bool res;
    res = wm.autoConnect("Motor_Driver"); // password protected ap ,"password"

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :):):)");
        reconnect(); //connect MQTT
        
    }
    client.subscribe("motor_velocities");
    //motor_velocities.setCallback(callback);
    

}

void loop() {
    // put your main code here, to run repeatedly:  

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  
  

}


//////////////////////////////////////////////////////////////////////
void saveParamCallback(){
  Serial.println("[CALLBACK] saveParamCallback fired");
  String server_temp = getParam("mqtt_server");
  String port_temp = getParam("mqtt_port");  
  server_temp.toCharArray(mqtt_server, server_temp.length() + 1);
  port_temp.toCharArray(mqtt_port, port_temp.length() + 1);
  Serial.print("PARAM mqtt_server = " + server_temp );
  Serial.println("PARAM mqtt_port = " +  port_temp);

}
/////////////////////////////////////////////////////////////////////////
String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(name == "mqtt_server") value = custom_mqtt_server.getValue();
  else{   if(name =="mqtt_port") value = custom_mqtt_port.getValue();  }
  return value;
}
//////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  char message[20];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message[i]=(char)payload[i];
  }
  Serial.println("");
  byte index = 0;
  ptr = strtok(message, ",");  // delimiter
  while (ptr != NULL)
   {
      strings[index] = ptr;
      index++;
      ptr = strtok(NULL, ",");
   }
  for (int n = 0; n < index; n++)
   {
      Serial.print(n);
      Serial.print("  ");
      Serial.println(strings[n]);
   }
  
}
////////////////////////////////////////////////////////////////////////////
void reconnect() {
   int8_t ret;

  // Stop if already connected.
  if (client.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 5;
  while (!client.connected()) { // connect will return 0 for connected
       if (client.connect("ESP32_clientID")) {
          Serial.println("connected");
          // Once connected, publish an announcement...
          client.publish("outTopic", "MotorDriver connected to MQTT");
          // ... and resubscribe
          client.subscribe("motor_velocities");
        }
        else {
          Serial.println("Retrying MQTT connection in 2 seconds...");
          delay(2000);  // wait 2 seconds
          retries--;
          if (retries == 0) {
            // basically die and wait for WDT to reset me
            //wm.resetSettings();
            break;
          }
        
        }
       
  }  
  if (!client.connected()) {
      Serial.println("Failed to connect MQTT - restarting ESP!");
      ESP.restart();
  }
  else {
      Serial.println("connected**");
  }
}
///////////////////////////////////////////////////////////////////////