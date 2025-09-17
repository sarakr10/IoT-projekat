#include <iostream>
#include <cstring>
#include <mosquitto.h>



//Definisati MQTT broker adresu i port
#define MQTT_SERVER_ADDRESS "localhost"
#define MQTT_SERVER_PORT 1883



//Definisati MQTT topike za senzore i aktuarore
#define SENSORS "sensors/+" //nema ove linije za aktuatore jer oni nisu subscribovani, ne primamo podatke sa njih, ne objabvljuju na topic
#define TEMPERATURE_TOPIC "sensors/temperature"
#define HEART_RATE_TOPIC "sensors/heart_rate"

#define EMERGENCY_CALL_TOPIC "actuators/emergency_call_module"
#define MACHINE_SHUTDOWN_TOPIC "actuators/shutdown_relay"

using namespace std;

//komande za aktuatore
#define COMMAND_ON "ON"
#define COMMAND_OFF "OFF"


// MQTT callback funkcija
void on_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    if (message->payloadlen) {
        std::cout << "Poruka je primljena na topic " << message->topic << std::endl;
        std::cout << "Payload: " << (char*)message->payload << std::endl;

        
        //Obrada primljene poruke
        if (strcmp(message->topic, TEMPERATURE_TOPIC) == 0) {
            double temperature = std::stod((char*)message->payload);
            const char* command = (temperature >= 38.5 || temperature <= 35.0) ? COMMAND_ON : COMMAND_OFF; 

            if (command) {
                
                //Publish komanda za iskljucivanje radnikove masine
                mosquitto_publish(mosq, NULL, MACHINE_SHUTDOWN_TOPIC, strlen(command), command, 1, true);
                // Output na temrinal
                std::cout << "\nObjavljena komanda " << command << " na topic: " << MACHINE_SHUTDOWN_TOPIC << std::endl;
            }
        }
         //Obrada primljene poruke
        if(strcmp(message->topic , HEART_RATE_TOPIC) == 0) {
            double heart_rate = stod((char*)message->payload);
            const char* command = (heart_rate >=105.0 || heart_rate <= 45.0) ? COMMAND_ON : COMMAND_OFF;

            if (command) {
                // Publish komanda za iskljucivanje radnikove masine i aktiviranje poziva hitne
                mosquitto_publish(mosq, NULL,  MACHINE_SHUTDOWN_TOPIC , strlen(command), command, 1, true);
                mosquitto_publish(mosq, NULL,  EMERGENCY_CALL_TOPIC , strlen(command), command, 1, true);

                // Output na terminal
                std::cout << "\nObjavljena komanda" << command << " na topic " <<  EMERGENCY_CALL_TOPIC << std::endl;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    struct mosquitto* mosq = NULL;

    // Inicijalizacija mosquitto biblioteke
    mosquitto_lib_init();

    
    //Kreirati instancu mosquitto klijenta
    mosq = mosquitto_new("controller_client", true, NULL);
    if (!mosq) {
        std::cerr << "Error: Neuspesno kreiranje instance mosquitto klijenta" << std::endl;
        mosquitto_lib_cleanup();
        return 1;
    }
  
  //Povezivanje na MQTT broker
    if (mosquitto_connect(mosq, MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT, 60) != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: Neuspesno povezivanje na MQTT broker" << std::endl;
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    cout << "Povezano na MQTT broker" << std::endl;

    // Subscribe na topic senzora
    mosquitto_subscribe(mosq, NULL, SENSORS, 1);
    cout << "Subscribovan na " << SENSORS " topic" << endl;

    
    //Postavka callback poruka
    mosquitto_message_callback_set(mosq, on_message_callback);

    //Mosquitto petlja
    mosquitto_loop_forever(mosq, -1, 1);

    
    //Prekid konekcije sa MQTT brokera
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    cout << "Prekid konekcije sa MQTT brokerom" << std::endl;

    return 0;
}

