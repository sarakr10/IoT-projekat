#include <iostream>
#include "httplib.h" // HTTP library za c++
#include <jsoncpp/json/json.h>
#include <sstream>
#define MQTT_SERVER_ADDRESS "localhost"




bool process_environment_json(const std::string& json_str) {
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(json_str);

    if (Json::parseFromStream(builder, s, &root, &errs)) {
        double temperature = root["temperature"].asDouble();
        double heart_rate = root["heart_rate"].asDouble();
        std::string shutdown = root["machine_shutdown_active"].asString();
        std::string emergency = root["emergency_call_active"].asString();

        std::cout << "Temperature: " << temperature << " °C\n";
        std::cout << "Heart rate: " << heart_rate << " bpm\n";
        std::cout << "Machine shutdown active: " << shutdown << "\n";
        std::cout << "Emergency call active: " << emergency << "\n";

        if(shutdown=="ON" || emergency=="ON"){
            return true;
        }else{
            return false;
        }
        
    } else {
        std::cerr << "Failed to parse JSON: " << errs << std::endl;
        return false;
    }

}


int main() {
    // Kreirati HTTP klijenta
    httplib::Client client("http://localhost:8080");
    
    //Slanje HTTP GET zahteva na server da bi se dobilo trenutno stanje okruzenja
    auto res = client.Get("/environment");
    // Provera da li je zahtev uspesno obradjen
    if (res && res->status == 200) {
        
        std::cout << std::endl;
        if(process_environment_json(res->body)){
            std::string input;
            do {
                std::cout << "Has the injured worker been replaced? (YES/NO)" << std::endl;
                std::cin >> input;
        
                // Pretvori unos u velika slova radi lakse provere
                for (auto & c: input) c = toupper(c);

                if(input == "NO") {
                    std::cout << "Worker has NOT been replaced!" << std::endl;
                } else if(input != "YES") {
                    std::cout << "Please enter YES or NO." << std::endl;
                }
            } while(input != "YES");
            
        }
    } else {
        std::cout << std::endl;
        std::cerr << "Error: Failed to retrieve status.\n";
        std::cout << std::endl;
        return 1;
    }

    
    return 0;
}
