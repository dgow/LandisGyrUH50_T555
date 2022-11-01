#include "esphome.h"


#define RXD2 16
#define TXD2 17

// Polling Intervall in minutes
int pollingInterval = 1;


class SensorMeterHeating : public PollingComponent {
  public:
  Sensor *sensor_kWh = new Sensor();
  Sensor *sensor_m3 = new Sensor();

  SensorMeterHeating() : PollingComponent(pollingInterval*60000) { }

  float get_setup_priority() const override { return esphome::setup_priority::HARDWARE; }


  void setup() override {
    
  }

  void update() override {
    Serial2.begin(300, SERIAL_7E1, RXD2, TXD2);
    Serial2.setTimeout(1000);
    
    // send init message
    ESP_LOGD("custom", "send init message");
    for (int i=0; i <= 40; i++){
        Serial2.write(0x00);
    }
  
    Serial2.write("\x2F\x3F\x21\x0D\x0A");
  
    // send request message
    //Serial2.write("/?!\x0D\x0A");
    Serial2.write("\x0D\x0A");
  
    Serial2.flush();
    delay(500);
    

    String IdMsg = Serial2.readStringUntil('\n');
    char IdMsg_c[IdMsg.length()+1] = {0};
    IdMsg.toCharArray(IdMsg_c, IdMsg.length());
    ESP_LOGD("custom", "Identification Message: %s", IdMsg_c);
    
    ESP_LOGD("custom", "Change Baudrate to 2400");
    Serial2.begin(2400, SERIAL_7E1, RXD2, TXD2);
    
    String output = Serial2.readStringUntil('\n');

    String kWh = substringBetween(output, "6.8(", "*MWh)6.26(");
    float sensorOutput_kWh = kWh.toFloat();

    String m3 = substringBetween(output, "6.26(", "*m3)9.21(");
    float sensorOutput_m3 = m3.toFloat();
    
    ESP_LOGD("custom", "Sensor loaded");
    
    //publish sensors if not 0
    if (sensorOutput_kWh != 0 && sensorOutput_m3 != 0) {
        ESP_LOGD("custom", "send data");
        sensor_kWh->publish_state(sensorOutput_kWh);
        sensor_m3->publish_state(sensorOutput_m3);
    }

//DEBUG

    ESP_LOGD("custom", "> %s >%s", kWh.c_str(), m3.c_str() );
    ESP_LOGD("custom", "======================");
    
    char msg[output.length() + 1] = {0};
    output.toCharArray(msg, output.length());
    
    ESP_LOGD("custom", "Message Length: %d", output.length());
    ESP_LOGD("custom", "Message : %s", msg);
    ESP_LOGD("custom", "======================");
    
// loop for all other lines
//for(int i = 0; i < 4; i++)
//{
//  String line = Serial2.readStringUntil('\n');
  //if(line.indexOf('!') != -1)
    //break;

//  ESP_LOGD("custom", "Line: %d. Message : %s", i, STR(line));
//}


    Serial2.flush();
    Serial2.end();  
  }
  
  char* STR(String s)
  {
    char* msg = new char[s.length() + 1];
    s.toCharArray(msg, s.length()); 
    return std::move(msg);
  }

  String substringBetween(String s, String a, String b) {
    int p = s.indexOf(a) + a.length();
    return s.substring(p, s.indexOf(b, p));
  }
};
