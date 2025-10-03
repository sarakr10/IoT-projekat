#include <iostream>
#include <cstring>
#include <mosquitto.h>
#include "httplib.h" 

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

// Globalne varijable za čuvanje stanja oba senzora
double last_temperature = 0.0;
double last_heart_rate = 0.0;
bool temp_received = false;
bool heart_received = false;

// MQTT callback funkcija
void on_message_callback(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    if (message->payloadlen) {

        std::cout<<std::endl;
        std::cout << "Received message on topic " << message->topic << std::endl;
        std::cout<<std::endl;
        std::cout << "Payload: " << (char*)message->payload << std::endl;
        std::cout<<std::endl;
        
        // Ažuriraj odgovarajući senzor
        if (strcmp(message->topic, TEMPERATURE_TOPIC) == 0) {
            last_temperature = std::stod((char*)message->payload);
            temp_received = true;
        }
        else if(strcmp(message->topic, HEART_RATE_TOPIC) == 0) {
            last_heart_rate = std::stod((char*)message->payload);
            heart_received = true;
        }
        
        // ANALIZA SAMO KADA SU OBA SENZORA DOBJENA
        if (temp_received && heart_received) {
            
            std::cout << "=== SYSTEM STATE ANALYSIS ===" << std::endl;
            std::cout << "Temperature: " << last_temperature << " °C" << std::endl;
            std::cout << "Heart rate: " << last_heart_rate << " bpm" << std::endl;
            
            // Proverava da li su senzori u alarmnom stanju
            bool temp_alarm = (last_temperature >= 38.5 || last_temperature <= 35.0);
            bool heart_alarm = (last_heart_rate >= 105.0 || last_heart_rate <= 45.0);
            
            std::cout << "Temperature alarm: " << (temp_alarm ? "YES" : "NO") << std::endl;
            std::cout << "Heart rate alarm: " << (heart_alarm ? "YES" : "NO") << std::endl;
            std::cout << std::endl;
            
            // GLAVNA LOGIKA NA OSNOVU OBA SENZORA
            if (temp_alarm && heart_alarm) {
                // OBA LOŠA - kritično stanje
                std::cout << "CRITICAL: Both sensors alarming!" << std::endl;
                mosquitto_publish(mosq, NULL, MACHINE_SHUTDOWN_TOPIC, strlen(COMMAND_ON), COMMAND_ON, 1, true);
                mosquitto_publish(mosq, NULL, EMERGENCY_CALL_TOPIC, strlen(COMMAND_ON), COMMAND_ON, 1, true);
                std::cout << "Published: Machine ON, Emergency ON" << std::endl;
                
            } else if (temp_alarm && !heart_alarm) {
                // TEMPERATURA LOŠA, PULS DOBAR
                std::cout << "WARNING: Temperature alarm only" << std::endl;
                mosquitto_publish(mosq, NULL, MACHINE_SHUTDOWN_TOPIC, strlen(COMMAND_ON), COMMAND_ON, 1, true);
                mosquitto_publish(mosq, NULL, EMERGENCY_CALL_TOPIC, strlen(COMMAND_OFF), COMMAND_OFF, 1, true);
                std::cout << "Published: Machine ON, Emergency OFF" << std::endl;
                
            } else if (!temp_alarm && heart_alarm) {
                // TEMPERATURA DOBRA, PULS LOŠ
                std::cout << "CRITICAL: Heart rate alarm only" << std::endl;
                mosquitto_publish(mosq, NULL, MACHINE_SHUTDOWN_TOPIC, strlen(COMMAND_ON), COMMAND_ON, 1, true);
                mosquitto_publish(mosq, NULL, EMERGENCY_CALL_TOPIC, strlen(COMMAND_ON), COMMAND_ON, 1, true);
                std::cout << "Published: Machine ON, Emergency ON" << std::endl;
                
            } else {
                // OBA DOBRA - sve normalno
                std::cout << "NORMAL: All sensors within normal range" << std::endl;
                mosquitto_publish(mosq, NULL, MACHINE_SHUTDOWN_TOPIC, strlen(COMMAND_OFF), COMMAND_OFF, 1, true);
                mosquitto_publish(mosq, NULL, EMERGENCY_CALL_TOPIC, strlen(COMMAND_OFF), COMMAND_OFF, 1, true);
                std::cout << "Published: Machine OFF, Emergency OFF" << std::endl;
            }
            
            std::cout << "===============================" << std::endl;
            std::cout << std::endl;
            
            // RESETUJ FLAGOVE NAKON ANALIZE
            temp_received = false;
            heart_received = false;
            
        } else {
            if (temp_received && !heart_received) {
                std::cout << "Temperature received, waiting for heart rate..." << std::endl;
            } else if (!temp_received && heart_received) {
                std::cout << "Heart rate received, waiting for temperature..." << std::endl;  
            } else {
                std::cout << "Waiting for sensor data..." << std::endl;
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
        std::cout << std::endl;
        std::cerr << "Error: Failed to create instance of mosquitto client" << std::endl;
        std::cout << std::endl;
        mosquitto_lib_cleanup();
        return 1;
    }
  
  //Povezivanje na MQTT broker
    if (mosquitto_connect(mosq, MQTT_SERVER_ADDRESS, MQTT_SERVER_PORT, 60) != MOSQ_ERR_SUCCESS) {
        std::cout << std::endl;
        std::cerr << "Error: Failed to connect to MQTT broker" << std::endl;
       std::cout << std::endl;
        mosquitto_destroy(mosq);
        mosquitto_lib_cleanup();
        return 1;
    }
    std::cout << std::endl;
    cout << "Connected to MQTT broker" << std::endl;
    std::cout << std::endl;

    // Subscribe na topic senzora
    mosquitto_subscribe(mosq, NULL, SENSORS, 1);
    std::cout << std::endl;
    cout << "Subscribed to " << SENSORS << " topic" << endl;
    std::cout << std::endl;
    
    //Postavka callback poruka
    mosquitto_message_callback_set(mosq, on_message_callback);

    //Mosquitto petlja
    mosquitto_loop_forever(mosq, -1, 1);

    
    //Prekid konekcije sa MQTT brokera
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    std::cout << std::endl;
    cout << "Disconnected from MQTT broker" << std::endl;
    std::cout << std::endl;
    return 0;
}