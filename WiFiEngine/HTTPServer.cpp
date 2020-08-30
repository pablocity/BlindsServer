#include <stdio.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define HTTP_REST_PORT 80
#define WIFI_RETRY_DELAY 500
#define MAX_WIFI_INIT_RETRY 50

enum State
{
    STOPPED,
    RUNNING_LEFT,
    RUNNING_RIGHT
};

class EngineState {

public:
    State state;
    int speed;

    EngineState(State state, int speed)
    {
        this->state = state;
        this->speed = speed;
    }
};

class HTTPServer
{
public:
    EngineState engine = EngineState(STOPPED, 0);

    HTTPServer(ESP8266WiFiClass wifi)
    {
        this->customWifi = wifi;
    }

    bool SetupServer()
    {
        if (init_wifi() == WL_CONNECTED)
        {
            Serial.println("Device is connected to the network");
            Serial.print("Server will run on ");
            Serial.print(customWifi.localIP());
            Serial.print("\n");
        }
        else
        {
            Serial.println("Device is not connected to a network");
            return false;
        }

        config_rest_server_routing();

        http_rest_server.begin();
        Serial.println("HTTP Server is running on port 80");

        return true;
    }

    void HandleClient()
    {
        http_rest_server.handleClient();
    }

private:

    String states[3] = { "STOPPED", "RUNNING_LEFT", "RUNNING_RIGHT" };

    ESP8266WiFiClass customWifi;

    ESP8266WebServer http_rest_server = { HTTP_REST_PORT };
    
    int init_wifi() {
        int retries = 0;

        customWifi.mode(WIFI_STA);
        //WiFi.begin(wifi_ssid, wifi_passwd);
        // check the status of WiFi connection to be WL_CONNECTED
        while ((customWifi.status() != WL_CONNECTED) && (retries < MAX_WIFI_INIT_RETRY)) {
            retries++;
            delay(WIFI_RETRY_DELAY);
            Serial.print("#");
        }
        return WiFi.status(); // return the WiFi connection status
    }

    void get_engine_state() {
        StaticJsonDocument<200> jsonDocument;
        char JSONmessageBuffer[200];
        jsonDocument["state"] = states[engine.state];
        jsonDocument["speed"] = engine.speed;
        serializeJsonPretty(jsonDocument, JSONmessageBuffer, sizeof(JSONmessageBuffer));
        http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
        http_rest_server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token");
        http_rest_server.send(200, "application/json", JSONmessageBuffer);
    }

    void json_to_resource(JsonDocument& jsonBody) {
        
        engine.state = jsonBody["state"];
        engine.speed = jsonBody["speed"];

        Serial.println(states[engine.state]);
        Serial.println(engine.speed);
    }

    void put_engine_state() {
        Serial.println("PUT engine state");
        StaticJsonDocument<500> jsonDocument;
        String post_body = http_rest_server.arg("plain");
        Serial.println(post_body);

        auto error = deserializeJson(jsonDocument, http_rest_server.arg("plain"));

        Serial.print("HTTP Method: ");
        Serial.println(http_rest_server.method());

        if (error) {
            Serial.println(error.c_str());
            Serial.println("error in parsing json body");
            http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
            http_rest_server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
            http_rest_server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token");
            http_rest_server.send(400);
        }
        else {
            if (http_rest_server.method() == HTTP_PUT) {
                Serial.println("PUT METHOD");
                json_to_resource(jsonDocument);
                http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
                http_rest_server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
                http_rest_server.sendHeader("Access-Control-Allow-Headers", "*");
                //http_rest_server.sendHeader("Location", "/engine/");
                http_rest_server.send(200);
            }
            else
            {
                Serial.println("Wrong method");
                http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
                http_rest_server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
                http_rest_server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token");
                http_rest_server.send(405);
            }
        }
    }

    void config_rest_server_routing() {
        http_rest_server.on("/", HTTP_GET, [&]() {
            http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
            http_rest_server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
            http_rest_server.sendHeader("Access-Control-Allow-Headers", "Origin, Content-Type, X-Auth-Token");
            http_rest_server.send(200, "text/html",
                "Welcome to the ESP8266 REST Web Server");
            });

        http_rest_server.on("/engine_state", HTTP_GET, [&]()
            {
                Serial.println("GET METHOD");
                get_engine_state();
            });
        http_rest_server.on("/engine_state", HTTP_PUT, [&]()
            {
                Serial.println("PUT METHOD ON");
                put_engine_state();
            });
        http_rest_server.on("/engine_state", HTTP_OPTIONS, [&]()
            {
                Serial.println("OPTIONS METHOD ON");
                http_rest_server.sendHeader("Access-Control-Allow-Origin", "*");
                http_rest_server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
                http_rest_server.sendHeader("Access-Control-Allow-Headers", "*");
                http_rest_server.send(204);

            });
    }

};


//void setup(void) {
//    Serial.begin(115200);
//
//    init_led_resource();
//    if (init_wifi() == WL_CONNECTED) {
//        Serial.print("Connetted to ");
//        Serial.print(wifi_ssid);
//        Serial.print("--- IP: ");
//        Serial.println(WiFi.localIP());
//    }
//    else {
//        Serial.print("Error connecting to: ");
//        Serial.println(wifi_ssid);
//    }
//
//    config_rest_server_routing();
//
//    http_rest_server.begin();
//    Serial.println("HTTP REST Server Started");
//}
//
//void loop(void) {
//    http_rest_server.handleClient();
//}
