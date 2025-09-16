#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <jsoncpp/json/json.h> // JSON library
#include <httplib.h> // HTTP library (make sure you have httplib.h)

using namespace std;

struct EnvironmentState {
    double temperature;
    double heart_rate;
    
    string emergency_call_active;
    string machine_shutdown_active;

    bool worker_replaced;

};


void simulateEnvironment(EnvironmentState& state) {

    while (true) {

        //temperatura moze da se promeni za maksimalno 1 stepen po iteraciji
        state.temperature += ((rand() % 5) - 2) * 0.5; // random +/- 1°C

        //otkucaji srca mogu da se promene za maksimalno 2bpm po iteraciji
        state.heart_rate += ((rand() % 5) - 2); // random +/- 2 bpm
    

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
       

        std::ofstream file("construction_site.json");
        file << root;
        file.close();
        
        // Print the current state
        std::cout << "Temperature: " << state.temperature << " °C" <<std::endl;
        std::cout << "Heart rate: " << state.heart_rate << " rpm" << std::endl;
        std::cout << "Machine shutdown module : " << state.machine_shutdown_active <<  std::endl;
        std::cout << "Emergency call active : " << state.emergency_call_active <<  std::endl;
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


    //DOVDE SAM STIGLA
    svr.Post("/update_relay_state", [&state](const httplib::Request& req, httplib::Response& res) {
    
        //parametar poruke koja se salje iz aktuatora
        //params.emplace("emergency_call_module", state);

     std::string response_msg;
    if (req.has_param("emergency_call_module")) {
        state.emergency_call_active = req.get_param_value("emergency_call_module");
        response_msg += "Emergency call module state updated. ";
    }
    if (req.has_param("shutdown_relay")) {
        state.machine_shutdown_active = req.get_param_value("shutdown_relay");
        response_msg += "Shutdown relay state updated.";
    }
    if (response_msg.empty()){
        response_msg = "Missing state parameters.";
    }
        res.set_content(response_msg, "text/plain");

        
    });

    svr.listen("0.0.0.0", 8080); 
}

int main() {
    EnvironmentState environmentState;
    environmentState.temperature = 36.5; // Initial temperature in Celsius
    environmentState.heart_rate = 70;
    environmentState.emergency_call_active = "OFF";
    environmentState.machine_shutdown_active = "OFF";

    std::thread simulation_thread(simulateEnvironment, std::ref(environmentState));
    startHttpServer(environmentState);
    
    simulation_thread.join();
    return 0;
}

