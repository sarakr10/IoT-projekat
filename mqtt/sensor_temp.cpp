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
    mosquitto_lib_init();//pre new pozvati
    struct mosquitto *mosq = mosquitto_new("temperature_sensor", true, NULL);
    if (!mosq) {
        std::cout<<std::endl;
        std::cerr << "Error: Unable to initialize MQTT client.\n";
        std::cout<<std::endl;
        return 1;
    }

    if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        std::cout<<std::endl;
        std::cerr << "Error: Unable to connect to MQTT broker.\n";
        std::cout<<std::endl;
        return 1;
    }

    httplib::Client cli("http://localhost:8080");

    while (true) {
        //menja endpoint prema HTTP serveru
        auto res = cli.Get("/environment");
        if (!res || res->status != 200) {
            std::cout<<std::endl;
            std::cerr << "Error: Unable to fetch worker data.\n";
            std::cout<<std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream s(res->body);
        if (!Json::parseFromStream(reader, s, &root, &errs)) {
            std::cout<<std::endl;
            std::cerr << "Error: Unable to parse JSON response.\n";
            std::cout<<std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        //zavisi od toga kako server šalje vrednost
        double temperature = root["temperature"].asDouble();

        std::string payload = std::to_string(temperature);
        mosquitto_publish(mosq, NULL, topic_temperature,
                          payload.length(), payload.c_str(), 0, false);

                          /*root je ceo JSON objekat koji je server poslao.
                            iz njega vadimo vrednost pod ključem "worker_temperature".
                            ta vrednost se pretvara u double i čuva u promenljivoj worker_temperature.*/
                            
        std::cout<<std::endl;
        std::cout << "Published worker temperature: "
                  << temperature << " °C\n";
        std::cout<<std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(3)); // periodično slanje
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
