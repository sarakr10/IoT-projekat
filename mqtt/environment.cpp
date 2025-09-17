#include <iostream>
#include <fstream>
#include <cmath>
#include <chrono>
#include <thread>
#include <jsoncpp/json/json.h> // JSON library
#include "httplib.h" // HTTP library (make sure you have httplib.h)

using namespace std;

struct EnvironmentState {
    double temperature;
    double heart_rate;

    string emergency_call_active;
    string machine_shutdown_active;

};

void simulateEnvironment(EnvironmentState& state) {
    // simulira parametre
    while (true) {
       static int t = 0;
        state.temperature = 36.5 + 2.0 * sin(t * 0.1); // promena ±2°C
        state.heart_rate = 100 + 5.0 * sin(t * 0.2);    // promena ±5 bpm
        t++;

        if(state.temperature >= 38.5){
            state.machine_shutdown_active = "ON";
            if(state.heart_rate >=105 || state.heart_rate <=45){
                 state.emergency_call_active = "ON";
            }
       }else if(state.temperature<=35){
            state.machine_shutdown_active = "ON";
            if(state.heart_rate >=105 || state.heart_rate <=45){
                 state.emergency_call_active = "ON";
            }
       }else{
            state.machine_shutdown_active = "OFF";
            state.emergency_call_active = "OFF";
       }


        Json::Value root;
        root["temperature"] = state.temperature;
        root["heart_rate"] = state.heart_rate;
        root["machine_shutdown_active"] = state.machine_shutdown_active;
        root["emergency_call_active"] = state.emergency_call_active;

        std::ofstream file("environment.json");
        file << root;
        file.close();
        
        //Ispis trenutnog stanja
        std::cout << "Temperature: " << state.temperature << " °C" <<std::endl;
        std::cout << "Heart rate: " << state.heart_rate << " bpm" << std::endl;
        std::cout << "Machine Shutdown Relay: " << state.machine_shutdown_active << std::endl;
        std::cout << "Emergency call active " << state.emergency_call_active << std::endl;
        std::cout << "*********************************" << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(3)); 
    }
}

void startHttpServer(EnvironmentState& state) {
    httplib::Server svr;

    svr.Get("/environment", [&state](const httplib::Request& req, httplib::Response& res) {
        Json::Value root;
        root["temperature"] = state.temperature;
        root["heart_rate"] = state.heart_rate;
        root["machine_shutdown_active"] = state.machine_shutdown_active;
        root["emergency_call_active"] = state.emergency_call_active;

        Json::StreamWriterBuilder writer;
        std::string output = Json::writeString(writer, root);
        res.set_content(output, "application/json");
    });

    svr.Post("/update_relay_state", [&state](const httplib::Request& req, httplib::Response& res) {
        //Stanje ventilatora
        if (req.has_param("emergency_call_module")) {
            state.relay_vent_state = req.get_param_value("emergency_call_module");
            res.set_content("Emergency call module state updated", "text/plain");
        } else {
            res.set_content("Missing emergency call state parameter", "text/plain");
        }
        //Stanje pumpe
        if (req.has_param("shutdown_relay")) {
            state.relay_pump_state = req.get_param_value("pump");
            res.set_content("Machine shutdown relay vent state updated", "text/plain");
        } else {
            res.set_content("Missing machine shutdown relay state parameter", "text/plain");
        }
    });

    svr.listen("0.0.0.0", 8080); // Ensure correct port
}

int main() {
    EnvironmentState environmentState;
    environmentState.temperature = 36.5; // Initial temperature in Celsius
    environmentState.heart_rate = 100.0; // Initial humidity percentage
    environmentState.emergency_call_active = "OFF";
    environmentState.machine_shutdown_active = "OFF";

    std::thread simulation_thread(simulateEnvironment, std::ref(environmentState));
    startHttpServer(environmentState);
    
    simulation_thread.join();
    return 0;
}
