#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <jsoncpp/json/json.h>
#include <cstdlib>
#include "httplib.h"

using namespace std;

struct EnvironmentState {
    double temperature;
    double heart_rate;
    string emergency_call_active;
    string machine_shutdown_active;
    bool worker_replaced;
};

void simulateEnvironment(EnvironmentState& state) {
    int temp_counter = 0;
    int heart_counter = 0;
   
    while (true) {
        // TEMPERATURA - redom kroz vrednosti
        double temp_values[] = {36.5, 37.0, 37.5, 38.0, 38.5, 39.0, 39.5, 40.0, 35.5, 35.0, 34.5};
        int temp_size = sizeof(temp_values) / sizeof(temp_values[0]);
        
        state.temperature = temp_values[temp_counter % temp_size];
        temp_counter++;
        
        // PULS - redom kroz vrednosti  
        double heart_values[] = {75, 80, 85, 90, 95, 100, 105, 110, 115, 50, 45, 40};
        int heart_size = sizeof(heart_values) / sizeof(heart_values[0]);
        
        state.heart_rate = heart_values[heart_counter % heart_size];
        heart_counter++;

        // state.temperature = 33.0 + (rand() % 801) / 100.0;  // 33.0 - 41.0°C
        // state.heart_rate = 40 + (rand() % 76);              // 40 - 115 bpm

        // LOGIKA ALARMA
        state.machine_shutdown_active = "OFF";
        state.emergency_call_active = "OFF";

        if (state.temperature >= 38.5 || state.temperature <= 35.0) {
            state.machine_shutdown_active = "ON";
        }

        if (state.heart_rate >= 105 || state.heart_rate <= 45) {
            state.emergency_call_active = "ON";
            state.machine_shutdown_active = "ON";
        }

        // GENERIŠ JSON FAJL - ATOMIC WRITE
        Json::Value root;
        root["temperature"] = state.temperature;
        root["heart_rate"] = state.heart_rate;
        root["machine_shutdown_active"] = state.machine_shutdown_active;
        root["emergency_call_active"] = state.emergency_call_active;

        std::ofstream tempFile("construction_site.json.tmp");
        tempFile << root;
        tempFile.close();
        std::rename("construction_site.json.tmp", "construction_site.json");
        
        // Print stanje
        std::cout << std::endl;
        std::cout << "*********************************" << std::endl;
        std::cout << "Temperature: " << state.temperature << " °C (step " << temp_counter << ")" << std::endl;
        std::cout << "Heart rate: " << state.heart_rate << " bpm (step " << heart_counter << ")" << std::endl;
        std::cout << "Machine shutdown: " << state.machine_shutdown_active << std::endl;
        std::cout << "Emergency call: " << state.emergency_call_active << std::endl;
        std::cout << "*********************************" << std::endl;
        std::cout << std::endl;

        // MAIN TIMING - 3 SEKUNDE
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

void startHttpServer(EnvironmentState& state) {
    httplib::Server svr;

    svr.Get("/environment", [&state](const httplib::Request& req, httplib::Response& res) {
        std::ifstream file("construction_site.json");
        if (!file.is_open()) {
            res.status = 500;
            res.set_content("Error: cannot open JSON file", "text/plain");
            return;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "application/json");
    });

    svr.Post("/update_relay_state", [&state](const httplib::Request& req, httplib::Response& res) {
        std::string response_msg = "";
        
        if (req.has_param("emergency_call_module")) {
            state.emergency_call_active = req.get_param_value("emergency_call_module");
            response_msg += "Emergency call updated. ";
        }
        if (req.has_param("shutdown_relay")) {
            state.machine_shutdown_active = req.get_param_value("shutdown_relay");
            response_msg += "Shutdown relay updated. ";
        }
        if (response_msg.empty()) {
            response_msg = "Missing parameters.";
        }
        
        res.set_content(response_msg, "text/plain");
    });

    std::cout << "HTTP Server starting on port 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080); 
}

int main() {
    //srand(time(0)); 
    EnvironmentState environmentState;
    environmentState.temperature = 36.5; 
    environmentState.heart_rate = 75;
    environmentState.emergency_call_active = "OFF";
    environmentState.machine_shutdown_active = "OFF";
    environmentState.worker_replaced = false;

    std::thread simulation_thread(simulateEnvironment, std::ref(environmentState));
    startHttpServer(environmentState);
    
    simulation_thread.join();
    return 0;
}
