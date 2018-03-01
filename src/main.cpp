// /*
// It connects to an MQTT server then:
// - on 0 switches off relay
// - on 1 switches on relay
// - on 2 toggles the state of the relay

// - sends 0 on off relay
// - sends 1 on on relay

// It will reconnect to the server if the connection is lost using a blocking
// reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
// achieve the same result without blocking the main loop.

// The current state is stored in EEPROM and restored on bootup
// */

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MqttRelay.h>
// #include <RelayButton.h>

#define espDeviceName "EspNo1"

const char *ssid = "grycownia";
const char *password = "ulaslodziuchnaaa";
const char *mqtt_server = "192.168.0.201";
const uint16_t mqtt_port = 1883;
const char *mqtt_user = "openHabEspNo1";
const char *mqtt_pwd = "habopen";

WiFiClient espWifiClient;
PubSubClient espMqttClient(espWifiClient);
long lastMsg = 0;
char msg[50];
char tmp_buffer[50];
//int value = 0;
const int numRelays = 2;
const char *debugTopic = "/myhome/Esp01/debug";

int esp12_internal_LED = 13;

int button1_pin = 0;
int button2_pin = 10;

int relay1_pin = 2;
int relay2_pin = 12;

MqttRelay relay1 = MqttRelay(relay1_pin, 1, "relay1", "/myhome/Esp01/btn1/command1", "/myhome/Esp01/btn1/state1", 0);
MqttRelay relay2 = MqttRelay(relay2_pin, 2, "relay2", "/myhome/Esp01/btn2/command1", "/myhome/Esp01/btn2/state1", 1);

MqttRelay Relays[] = {relay1, relay2};

void externalButtons();
void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();
void heartbeat(uint16_t delay);
void blink(int led, uint8_t times, uint16_t delay);

void setup()
{
    int debouncerInterval = 100;
    EEPROM.begin(512); // Begin eeprom to store on/off state
    for (int i = 0; i < numRelays; i++)
    {
        MqttRelay relay = Relays[i];
        RelayButton button = RelayButton(button1_pin, debouncerInterval);
        relay.attachButton(button);
        relay.init();
        relay.setState(EEPROM.read(relay.eepromAddress));
    }

    pinMode(esp12_internal_LED, OUTPUT);

    blink(esp12_internal_LED, 2, 500);

    Serial.begin(115200);
    setup_wifi(); // Connect to wifi
    espMqttClient.setServer(mqtt_server, mqtt_port);
    espMqttClient.setCallback(callback);
}

void loop()
{
    delay(50);
    if (!espMqttClient.connected())
    {
        reconnect();
    }
    espMqttClient.loop();
    externalButtons();
    heartbeat(10000);
}

//=================================================================================================================================

void setup_wifi()
{
    delay(100);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        externalButtons();
        for (int i = 0; i < 500; i++)
        {
            externalButtons();
            delay(1);
        }
        Serial.print(".");
    }
    blink(esp12_internal_LED, 3, 500);
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("\nMessage arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    MqttRelay relay;
    for (int i = 0; i < numRelays; i++)
    {
        if (topic == Relays[i].inTopic)
        {
            relay = Relays[i];
            return;
        }
    }

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0')
    {
        relay.turnOff();
        Serial.println(relay.name + " -> LOW");
        EEPROM.write(relay.eepromAddress, relay.getState()); // Write state to EEPROM
        EEPROM.commit();
        espMqttClient.publish(relay.outTopic, "0");
    }
    else if ((char)payload[1] == '1')
    {
        relay.turnOn();
        Serial.println(relay.name + " -> HIGH");
        EEPROM.write(relay.eepromAddress, relay.getState()); // Write state to EEPROM
        EEPROM.commit();
        espMqttClient.publish(relay.outTopic, "1");
    }
    //else if ((char)payload[0] == '2') {
    //	relay1State = !relay1State;
    //	digitalWrite(relay1_pin, relay1State);  // Turn the LED off by making the voltage HIGH
    //	Serial.print("relay1_pin -> switched to ");
    //	Serial.println(relay1State);
    //	EEPROM.write(0, relay1State);    // Write state to EEPROM
    //	EEPROM.commit();
    //	client.publish(outTopic, relay1State ? "1" : "0");
    //}
    else if ((char)payload[0] == '?')
    {
        // digitalWrite(relay1_pin, relay1State); // Turn the LED off by making the voltage HIGH
        Serial.print(relay.name + " current state -> ");
        Serial.println(relay.getState());
        espMqttClient.publish(relay.outTopic, "current state:");
        espMqttClient.publish(relay.outTopic, relay.getState() ? "1" : "0");
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!espMqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        // if (espMqttClient.connect("Esp01Client"))
        sprintf(tmp_buffer, "%sMqttClient", espDeviceName);
        if (espMqttClient.connect(tmp_buffer))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            sprintf(tmp_buffer, "%s booted succesfully", espDeviceName);
            // espMqttClient.publish(outTopic,  "Esp01 booted succesfully");
            espMqttClient.publish(debugTopic, tmp_buffer);
            // ... and resubscribe
            for (int i = 0; i < numRelays; i++)
            {
                espMqttClient.subscribe(Relays[i].inTopic);
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(espMqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            for (int i = 0; i < 5000; i++)
            {
                externalButtons();
                delay(1);
            }
        }
    }
}

void externalButtons()
{
    for (int i = 1; i < numRelays; i++)
    {
        MqttRelay relay = Relays[i];
        Bounce debouncer = relay.button.debouncer;
        debouncer.update();
        if (debouncer.fell())
        {
            Serial.println(relay.name + "debouncer fell");
            // Toggle relay state :
            relay.toggle();
            EEPROM.write(relay.eepromAddress, relay.getState()); // Write state to EEPROM
            espMqttClient.publish(relay.outTopic, relay.getState() ? "1" : "0");
        }
    }
}

void blink(int led, uint8_t times, uint16_t delay)
{
    long next;
    digitalWrite(esp12_internal_LED, HIGH); // Blink to indicate setup
    next = millis() + delay;
    while (millis() < next)
    {
    }
    digitalWrite(esp12_internal_LED, LOW);
    next = millis() + delay;
    while (millis() < next)
    {
    }
}

void heartbeat(uint16_t delay = 10000)
{
    long now = millis();
    if (now - lastMsg > delay)
    {
        lastMsg = now;
        espMqttClient.publish(debugTopic, "heartbeat");
        Serial.print("o");
    }
}