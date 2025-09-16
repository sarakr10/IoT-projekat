#include <iostream>
#include <cpphttplib/httplib.h> // HTTP library za c++

int main() {
    // Kreirati HTTP klijenta
    httplib::Client client("http://localhost:8080");
    
    //Slanje HTTP GET zahteva na server da bi se dobilo trenutno stanje okruzenja
    auto res = client.Get("/environment");
    
    // Provera da li je zahtev uspesno obradjen
    if (res && res->status == 200) {
        
        // Ispis tela odgovora (stanje okruzenja)
        std::cout << "Trenutno stanje okruzenja:\n" << res->body << std::endl;
    } else {
        std::cerr << "Error: Neuspesno vracanje stanja.\n";
        return 1;
    }
    return 0;
}
