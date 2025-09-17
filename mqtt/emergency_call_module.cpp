#include <iostream>
#include <mosquitto.h>
#include <jsoncpp/json/json.h>
#include "httplib.h"

const char *mqtt_host = "localhost";        //broker
const int mqtt_port = 1883;                 //port na kom broker slusa

//topic_pump -> topic_emergency_call
const char *topic_emergency_call = "actuators/emergency_call_module";   //topic na kom slusa poruke za modul

//slusa mqtt poruke za emergency call modul i obavestava server

//informise server da li je modul ukljucen ili iskljucen
void notifyEnvironment(const std::string &state) {

    httplib::Client cli("http://localhost:8080");
    httplib::Params params;

    params.emplace("emergency_call_module", state);                 //kolekcija parova kljuc - vrednost
    auto res = cli.Post("/update_relay_state", params);
    if (res && res->status == 200) {
        std::cout<<std::endl;
        std::cout << "Notified environment: Emergency call module state " << state << std::endl;
    } else {
        std::cout<<std::endl;
        std::cerr << "Failed to notify environment" << std::endl;
    }
}

//reaguje na poruke sa brokera i prosledjuje stanje modula http serveru
void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    std::string payload(static_cast<char *>(message->payload), message->payloadlen);
     std::cout<<std::endl;
    std::cout << "Message received on topic: " << message->topic << std::endl;
    std::cout << "Payload: " << payload << std::endl;

    if (payload == "ON") {
        std::coout<<std::endl;
        std::cout << "Emergency Call Module actuator turned ON" << std::endl;
        notifyEnvironment("ON");
      
    } else if (payload == "OFF") {
        std::cout<<std::endl;
        std::cout << "Emergency Call Module acturator turned OFF" << std::endl;
        notifyEnvironment("OFF");
      
    }
}

int main() {
    mosquitto_lib_init();
    struct mosquitto *mosq = mosquitto_new(NULL, true, NULL);
    if (!mosq) {
        std::cerr << "Error: Unable to initialize MQTT client.\n";
        return 1;
    }

    mosquitto_message_callback_set(mosq, on_message);

    if (mosquitto_connect(mosq, mqtt_host, mqtt_port, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: Unable to connect to MQTT broker.\n";
        return 1;
    }

    mosquitto_subscribe(mosq, NULL, topic_emergency_call, 0);

    int loop = mosquitto_loop_forever(mosq, -1, 1);
    if (loop != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: " << mosquitto_strerror(loop) << std::endl;
        return 1;
    }

    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    return 0;
}

