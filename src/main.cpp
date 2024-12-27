#include <Arduino.h>
#include "Wire.h"
#include "SHT2x.h"

// Enable debug prints to serial monitor
//#define MY_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RF24
#define MY_RF24_PA_LEVEL RF24_PA_HIGH
#define MY_RF24_CHANNEL (125)
#define MY_RF24_DATARATE (RF24_1MBPS)

// Enabled repeater feature for this node
#define MY_REPEATER_FEATURE

#include <MySensors.h>

#define LOCK_DOORS_CHILD_ID   1
#define UNLOCK_DOOR_CHILD_ID 2
#define PANIC_DOOR_CHILD_ID  3
#define TEMPERATURE_CHILD_ID 4
#define HUMIDITY_CHILD_ID  5
#define UNLOCK_ALL_DOORS_CHILD_ID 7

#define LOCK_DOOR_PIN   3
#define UNLOCK_DOOR_PIN 4
#define PANIC_DOOR_PIN  5

#define NORMAL_BUTTON_PRESS_MS 750
#define LONG_BUTTON_PRESS_MS 1500
#define QUICK_BUTTON_PRESS_MS 250

#define SHT21_TEMPERATURE_OFFSET (-1.40)
#define SHT21_HUMIDITY_OFFSET (11.26)

#define HEARTBEAT_INTERVAL 60000

MyMessage lock_msg(LOCK_DOORS_CHILD_ID, V_STATUS);
MyMessage unlock_msg(UNLOCK_DOOR_CHILD_ID, V_STATUS);
MyMessage panic_msg(PANIC_DOOR_CHILD_ID, V_STATUS);
MyMessage temp_msg(TEMPERATURE_CHILD_ID, V_TEMP);
MyMessage hum_msg(HUMIDITY_CHILD_ID, V_HUM);
MyMessage unlock_all_msg(UNLOCK_ALL_DOORS_CHILD_ID, V_STATUS);
bool initialValueSent = false;

SHT2x sht;

unsigned long lastHeartbeat;

void before()
{
  digitalWrite(LOCK_DOOR_PIN, HIGH);
  digitalWrite(UNLOCK_DOOR_PIN, HIGH);
  digitalWrite(PANIC_DOOR_PIN, HIGH);
  pinMode(LOCK_DOOR_PIN, OUTPUT); 
  pinMode(UNLOCK_DOOR_PIN, OUTPUT); 
  pinMode(PANIC_DOOR_PIN, OUTPUT); 
}

void presentation()
{
    sendSketchInfo(F("Yaris Remote"), F("1.0"));

    present(LOCK_DOORS_CHILD_ID, S_BINARY, F("Lock"));
    present(UNLOCK_DOOR_CHILD_ID, S_BINARY, F("Unlock"));
    present(PANIC_DOOR_CHILD_ID, S_BINARY, F("Panic"));
    present(TEMPERATURE_CHILD_ID, S_TEMP, F("Temperature"));
    present(HUMIDITY_CHILD_ID, S_HUM, F("Humidty"));
    present(UNLOCK_ALL_DOORS_CHILD_ID, S_BINARY, F("Unlock All"));
}

void setup() 
{
  Wire.begin();
  sht.begin();
  sht.requestTemperature();
  lastHeartbeat = millis();
}

void loop() 
{
  char szWork[10];
  if(initialValueSent == false)
  {
    initialValueSent = true;
    send(lock_msg.set(LOW));
    wait(10, C_SET, V_STATUS);
    send(unlock_msg.set(LOW));
    wait(10, C_SET, V_STATUS);
    send(panic_msg.set(LOW));
    wait(10, C_SET, V_STATUS);
    send(unlock_all_msg.set(LOW));
    wait(1, C_SET, V_STATUS);
    sendHeartbeat();
  }

  if (sht.reqTempReady())
  {
    szWork[0] = '\0';
    sht.readTemperature();
    Serial.print("TEMP:\t");
    dtostrf(sht.getTemperature() + SHT21_TEMPERATURE_OFFSET, 2, 2, szWork);
    Serial.println(szWork);

    send(temp_msg.set(szWork));

    wait(30000);
    sht.requestHumidity();
  }

  if (sht.reqHumReady())
  {
    szWork[0] = '\0';
    sht.readHumidity();
    if(sht.getHumidity() > 5.0)
    {
      Serial.print("HUM:\t");
      dtostrf(sht.getHumidity() + SHT21_HUMIDITY_OFFSET, 2, 2, szWork);
      Serial.println(szWork);
      send(hum_msg.set(szWork));
    }

    wait(30000);
    sht.requestTemperature();
    initialValueSent = false;
  }

  if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) 
  {
      sendHeartbeat();
      lastHeartbeat = millis();
  }
}

void receive(const MyMessage &message)
{
    // We only expect one type of message from controller. But we better check anyway.
    if (message.getType() == V_STATUS) 
    {

      if(message.getBool() == HIGH)
      {
        switch(message.getSensor())
        {
          case LOCK_DOORS_CHILD_ID:
            digitalWrite(LOCK_DOOR_PIN, LOW);
            wait(NORMAL_BUTTON_PRESS_MS);
            digitalWrite(LOCK_DOOR_PIN, HIGH);
            send(lock_msg.set(LOW));
          break;
          case UNLOCK_DOOR_CHILD_ID:
            digitalWrite(UNLOCK_DOOR_PIN, LOW);
            wait(NORMAL_BUTTON_PRESS_MS);
            digitalWrite(UNLOCK_DOOR_PIN, HIGH);
            send(unlock_msg.set(LOW));
          break;
          case PANIC_DOOR_CHILD_ID:
            digitalWrite(PANIC_DOOR_PIN, LOW);
            wait(LONG_BUTTON_PRESS_MS);
            digitalWrite(PANIC_DOOR_PIN, HIGH);
            send(panic_msg.set(LOW));
          break;
            case UNLOCK_ALL_DOORS_CHILD_ID:
            digitalWrite(UNLOCK_DOOR_PIN, LOW);
            wait(QUICK_BUTTON_PRESS_MS);
            digitalWrite(UNLOCK_DOOR_PIN, HIGH);
            wait(QUICK_BUTTON_PRESS_MS);
            digitalWrite(UNLOCK_DOOR_PIN, LOW);
            wait(QUICK_BUTTON_PRESS_MS);
            digitalWrite(UNLOCK_DOOR_PIN, HIGH);
            send(unlock_all_msg.set(LOW));
          break;
          default:
            // Do nothing
          break;
        } 
      }
      
      // Write some debug info
      Serial.print("S:");
      Serial.print(message.getSensor());
      Serial.print(", ");
      Serial.println(message.getBool());
    }
}
