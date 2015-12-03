#include <ESPert.h>

ESPert espert;

IPAddress mqtt_server(192,168,77,1);
//char* mqtt_server = "mqtt.espert.io";

int currentSwitch = true;
String outTopic = "ESPert/" + String(espert.info.getChipId()) + "/LED";

String inTopic = "ESPert/" + String(espert.info.getChipId()) + "/LED";;

void callback(char* topic, byte* payload, unsigned int length) {
  byte* p = (byte*)malloc(length+1);
  // Copy the payload to the new buffer
  memcpy(p,payload,length);
  p[length] = 0;
  String strPayload = String((char *)p);
  free(p);
  
  espert.println("Receive: " + strPayload);
  String key = "cmd";

  if (espert.json.init(strPayload) && espert.json.containsKey(key)) {
  
    String value = espert.json.get(key);

    if (value == "0") {
      espert.led.off();
      espert.println("LED: Off");
    } else if (value == "1") {
      espert.led.on();
      espert.println("LED: On");
    }

    String outString  = "{\"status\":\"" + String(espert.led.isOn() ? 1 : 0) + "\", ";
    outString += "\"name\":\"" + String(espert.info.getId()) + "\"}";
    espert.println("Send...: " + outString);
    espert.mqtt.publish(outTopic, outString);
  }
}

void setup() {
  espert.init();
  espert.mqtt.init(mqtt_server, 1883, callback);

  espert.oled.init();
  delay(2000);

  espert.println("Press USER button to turn on LED.");

  espert.oled.clear();
  espert.oled.println(espert.info.getId());
  espert.oled.println();

  int mode = espert.wifi.init();

  if (mode == ESPERT_WIFI_MODE_CONNECT) {
    espert.println(">>> WiFi mode: connected.");
    espert.oled.println("WiFi: connected.");
    espert.oled.print("IP..: ");
    espert.oled.println(espert.wifi.getLocalIP());
  } else if (mode == ESPERT_WIFI_MODE_DISCONNECT) {
    espert.println(">>> WiFi mode: disconnected.");
    espert.oled.println("WiFi: not connected.");
  }
}

void loop() {
  if (espert.mqtt.connect()) {
    espert.println("MQTT: Connected");
    espert.println("MQTT: Out Topic " + outTopic);
    espert.mqtt.subscribe(inTopic);
    Serial.println("MQTT: Subscribed " + inTopic);
  }

  bool buttonPressed = espert.button.isOn();

  if (buttonPressed != currentSwitch) {
    String outString  = "{\"cmd\":\"" + String(buttonPressed ? 1 : 0) + "\", ";
    outString += "\"name\":\"" + String(espert.info.getId()) + "\"}";
    espert.println("Send...: " + outString);
    espert.mqtt.publish(outTopic, outString);
    currentSwitch = buttonPressed;
  }
}

