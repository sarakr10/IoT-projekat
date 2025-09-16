#include <iostream>
#include <mosquitto.h>
#include <jsoncpp/json/json.h>
#include <httplib.h>
#include <thread>
#include <chrono>
#include <sstream>

const char *mqtt_host = "localhost";
const int mqtt_port = 1883;
const char *topic_heart_rate = "sensors/heart_rate";

int main() {
    mosquitto_lib_init(); // inicijalizacija pre new
    struct mosquitto *mosq = mosquitto_new("worker_heart_rate_sensor", true, NULL);
    if (!mosq) {
        std::cerr << "Error: Unable to initialize MQTT client.\n";
        return 1;
    }

    if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: Unable to connect to MQTT broker.\n";
        return 1;
    }

    httplib::Client cli("http://localhost:8080");

    while (true) {
        // menjaj endpoint prema HTTP serveru
        auto res = cli.Get("/worker");
        if (!res || res->status != 200) {
            std::cerr << "Error: Unable to fetch worker data.\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        Json::Value root;
        Json::CharReaderBuilder reader;
        std::string errs;
        std::istringstream s(res->body);
        if (!Json::parseFromStream(reader, s, &root, &errs)) {
            std::cerr << "Error: Unable to parse JSON response.\n";
            std::this_thread::sleep_for(std::chrono::seconds(5));
            continue;
        }

        // zavisi od toga kako server šalje vrednost
        double worker_heart_rate = root["worker_heart_rate"].asDouble();

        std::string payload = std::to_string(worker_heart_rate);
        mosquitto_publish(mosq, NULL, topic_heart_rate,
                          payload.length(), payload.c_str(), 0, false);

        /* root je ceo JSON objekat koji je server poslao.
           iz njega vadimo vrednost pod ključem "worker_heart_rate".
           ta vrednost se pretvara u double i čuva u promenljivoj worker_heart_rate. */

        std::cout << "Published worker heart rate: "
                  << worker_heart_rate << " bpm\n";

        std::this_thread::sleep_for(std::chrono::seconds(5)); // periodično slanje
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}
