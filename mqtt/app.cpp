#include <iostream>
#include "httplib.h" // HTTP library za c++

int main() {
    // Kreirati HTTP klijenta
    httplib::Client client("http://localhost:8080");
    
    //Slanje HTTP GET zahteva na server da bi se dobilo trenutno stanje okruzenja
    auto res = client.Get("/environment");
    
    // Provera da li je zahtev uspesno obradjen
    if (res && res->status == 200) {
        
        std::cout << std::endl;
        // Ispis tela odgovora (stanje okruzenja)
        std::cout << "Current environment status:\n" << res->body << std::endl;
        std::cout << std::endl;
    } else {
        std::cout << std::endl;
        std::cerr << "Error: Failed to retrieve status.\n";
        std::cout << std::endl;
        return 1;
    }
    return 0;
}