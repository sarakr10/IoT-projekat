#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <jsoncpp/json/json.h>
#include <cstdlib>
#include <iomanip>
#include <signal.h>
#include <atomic>

#include "httplib.h"

using namespace std;

// GLOBAL FLAG ZA GRACEFUL SHUTDOWN
std::atomic<bool> keep_running{true};
httplib::Server* global_server = nullptr;

struct EnvironmentState {
    double temperature;
    double heart_rate;
    string emergency_call_active;
    string machine_shutdown_active;
    bool worker_replaced;
    int cycle_count;
};

// SIGNAL HANDLER ZA CTRL+C
void signal_handler(int signal) {
    //signal interrupt ili terminate
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << std::endl;
        std::cout << "SHUTDOWN SIGNAL RECEIVED" << std::endl;
        std::cout << "Cleaning up and stopping all processes..." << std::endl;
        
        keep_running = false;
        
        // Zaustavi HTTP server
        if (global_server) {
            global_server->stop();
        }
        
        // Obriši JSON fajlove
        std::remove("construction_site.json");
        std::remove("construction_site.json.tmp");
        
        std::cout << "Environment stopped cleanly!" << std::endl;
        std::exit(0);
    }
}

void simulateEnvironment(EnvironmentState& state) {
    int temp_counter = 0;
    int heart_counter = 0;
    state.cycle_count = 0;
   
    while (keep_running) {
        state.cycle_count++;
        
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

        // KALKULACIJA OCEKIVANIH VREDNOSTI AKTUATORA
        state.machine_shutdown_active = "OFF";
        state.emergency_call_active = "OFF";

        bool temp_alarm = (state.temperature >= 38.5 || state.temperature <= 35.0);
        bool heart_alarm = (state.heart_rate >= 105 || state.heart_rate <= 45);

        if (temp_alarm) {
            state.machine_shutdown_active = "ON";
        }

        if (heart_alarm) {
            state.emergency_call_active = "ON";
            state.machine_shutdown_active = "ON";
        }

        // UBACUHE VREDNOSTI U JSON FAJL
        Json::Value root;
        root["temperature"] = state.temperature;
        root["heart_rate"] = state.heart_rate;
        root["machine_shutdown_active"] = state.machine_shutdown_active;
        root["emergency_call_active"] = state.emergency_call_active;

        // SAFE FILE WRITE - pravi privremeni fajl koji postaje pravi ukoliko je uspesno napisan
        try {
            std::ofstream tempFile("construction_site.json.tmp");
            tempFile << root;
            tempFile.close();
            std::rename("construction_site.json.tmp", "construction_site.json");
        } catch (...) {
            std::cerr << "Warning: Could not write JSON file" << std::endl;
        }
        
        // ISPIS STANJA
        if (keep_running) {
            std::cout << std::endl;
            std::cout << "========================================" << std::endl;
            std::cout << "        ENVIRONMENT CYCLE #" << state.cycle_count << std::endl;
            std::cout << "========================================" << std::endl;
            
            // Temperature status
            std::cout << "Temperature: " << std::fixed << std::setprecision(1) << state.temperature << " °C ";
            if (temp_alarm) {
                std::cout << (state.temperature >= 38.5 ? "HIGH FEVER" : "HYPOTHERMIA");
            } else {
                std::cout << "Normal";
            }
            std::cout << std::endl;
            
            // Heart rate status  
            std::cout << "Heart rate: " << std::fixed << std::setprecision(0) << state.heart_rate << " bpm ";
            if (heart_alarm) {
                std::cout << (state.heart_rate >= 105 ? "TACHYCARDIA" : "BRADYCARDIA");
            } else {
                std::cout << "Normal";
            }
            std::cout << std::endl;
            
            std::cout << "----------------------------------------" << std::endl;
            
            // Machine Shutdown status
            std::cout << "Machine shutdown: " << state.machine_shutdown_active;
            if (state.machine_shutdown_active == "ON") {
                std::cout << "MACHINE STOPPED";
            } else {
                std::cout << "Machine running";
            }
            std::cout << std::endl;
            
            // Emergency Call status
            std::cout << "Emergency call: " << state.emergency_call_active;
            if (state.emergency_call_active == "ON") {
                std::cout << "EMERGENCY ACTIVE";
            } else {
                std::cout << "No emergency";
            }
            std::cout << std::endl;
            
            std::cout << "========================================" << std::endl;
            
            // Overall system status
            if (heart_alarm) {
                std::cout << "CRITICAL: Heart rate emergency - Worker needs immediate help!" << std::endl;
            } else if (temp_alarm) {
                std::cout << "WARNING: Temperature alarm - Worker should rest!" << std::endl;
            } else {
                std::cout << "NORMAL: All systems operating normally" << std::endl;
            }
            
            std::cout << std::endl;
        }

        // MAIN TIMING - 3 SEKUNDE sa interrupt check
        for (int i = 0; i < 30 && keep_running; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

void startHttpServer(EnvironmentState& state) {
    //lokalni server objekat
    httplib::Server svr;
    global_server = &svr;

    // endpotiny environment
    svr.Get("/environment", [&state](const httplib::Request& req, httplib::Response& res) {
        std::ifstream file("construction_site.json");
        if (!file.is_open()) {
            res.status = 500;
            res.set_content("Error: cannot open JSON file", "text/plain");
            return;
        }
        //procita ceo fajl i salje ga kao JSON odgovor
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        res.set_content(buffer.str(), "application/json");
    });
    // endpoint za update relays, omogucava daljinsko menjanje stanja releja
    svr.Post("/update_relay_state", [&state](const httplib::Request& req, httplib::Response& res) {
        std::string response_msg = "";
        
        if (req.has_param("emergency_call_module")) {
            state.emergency_call_active = req.get_param_value("emergency_call_module");
            response_msg += "Emergency call updated to " + state.emergency_call_active + ". ";
        }
        if (req.has_param("shutdown_relay")) {
            state.machine_shutdown_active = req.get_param_value("shutdown_relay");
            response_msg += "Shutdown relay updated to " + state.machine_shutdown_active + ". ";
        }
        if (response_msg.empty()) {
            response_msg = "Missing parameters.";
        }
        
        res.set_content(response_msg, "text/plain");
    });

    std::cout << "HTTP Server starting on port 8080..." << std::endl;
    std::cout << "Environment simulation will begin in 3 seconds..." << std::endl;
    std::cout << "Press Ctrl+C to stop gracefully" << std::endl;
    std::cout << std::endl;
    
    svr.listen("0.0.0.0", 8080); 
}

int main() {
    // REGISTRUJ SIGNAL HANDLERS
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // OBRIŠI STARE JSON FAJLOVE
    std::remove("construction_site.json");
    std::remove("construction_site.json.tmp");
    
    EnvironmentState environmentState;
    environmentState.temperature = 36.5; 
    environmentState.heart_rate = 75;
    environmentState.emergency_call_active = "OFF";
    environmentState.machine_shutdown_active = "OFF";
    environmentState.worker_replaced = false;
    environmentState.cycle_count = 0;

    std::thread simulation_thread(simulateEnvironment, std::ref(environmentState));
    startHttpServer(environmentState);
    
    simulation_thread.join();
    
    // CLEANUP
    std::remove("construction_site.json");
    std::remove("construction_site.json.tmp");
    
    return 0;
}
