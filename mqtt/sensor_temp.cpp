#include <iostream>
#include <mosquitto.h>
#include <jsoncpp/json/json.h>
#include "httplib.h"
#include <thread>
#include <chrono>
#include <sstream>

const char *mqtt_host = "localhost";
const int mqtt_port = 1883;
const char *topic_temperature = "sensors/temperature";

int main() {
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new("temperature_sensor", true, NULL);
    if (!mosq) {
        std::cerr << "Error: Unable to initialize MQTT client.\n";
        return 1;
    }

    if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: Unable to connect to MQTT broker.\n";
        return 1;
    }

    std::cout << "Temperature sensor connected!" << std::endl;
    httplib::Client cli("http://localhost:8080");

    // ČEKA PRVI CIKLUS ENVIRONMENT-A (1 sec stagger)
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while (true) {
        int retry_count = 0;
        bool success = false;
        
        while (!success && retry_count < 3) {
            auto res = cli.Get("/environment");
            
            if (!res || res->status != 200) {
                retry_count++;
                std::cerr << "TEMP Retry " << retry_count << " - Cannot fetch data.\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                continue;
            }

            try {
                Json::Value root;
                Json::CharReaderBuilder reader;
                std::string errs;
                std::istringstream s(res->body);
                
                if (!Json::parseFromStream(reader, s, &root, &errs)) {
                    retry_count++;
                    std::cerr << "TEMP Retry " << retry_count << " - JSON parse error\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    continue;
                }

                if (!root.isMember("temperature")) {
                    retry_count++;
                    std::cerr << "TEMP Retry " << retry_count << " - No temperature field\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    continue;
                }

                double temperature = root["temperature"].asDouble();
                
                std::string payload = std::to_string(temperature);
                int result = mosquitto_publish(mosq, NULL, topic_temperature,
                                  payload.length(), payload.c_str(), 0, false);
                
                if (result == MOSQ_ERR_SUCCESS) {
                    std::cout << "TEMP SENSOR: Published temperature: " << temperature << " °C" << std::endl;
                    success = true;
                } else {
                    retry_count++;
                    std::cerr << "TEMP Retry " << retry_count << " - MQTT publish error\n";
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                }
            }
            catch (const std::exception& e) {
                retry_count++;
                std::cerr << "TEMP Retry " << retry_count << " - Exception: " << e.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        }
        
        if (!success) {
            std::cerr << "Temperature sensor: Failed after 3 retries, skipping cycle" << std::endl;
        }

        // MAIN TIMING - 3 SEKUNDE (kao environment)
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}