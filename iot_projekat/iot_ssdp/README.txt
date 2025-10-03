Kontroler:
g++ -Wall -std=c++11 controller.cpp -o controller -lmosquitto -ljsoncpp -lpthread
./controller

Svaka od komponenti u sopstvenom direktorijumu:
g++ -Wall -std=c++11 device.cpp -o device -lmosquitto -ljsoncpp -lpthread
./device

