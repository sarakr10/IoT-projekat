!NAKON ZAVRSETKA PROGRAMA POKRENUTI
./cleanup.sh

KOMPAJLIRANJE I POKRETANJE SENZORA I AKTUATORA:
g++ -Wall -std=c++11 sensor_temp.cpp -o temp -lmosquitto -ljsoncpp -lpthread
./temp

g++ -Wall -std=c++11 heart_rate.cpp -o hr -lmosquitto -ljsoncpp -lpthread
./hr

g++ -Wall -std=c++11 machine_shutdown_merged.cpp -o ms -lmosquitto -ljsoncpp -lpthread
./ms

g++ -Wall -std=c++11 emergency_call_merged.cpp -o ec -lmosquitto -ljsoncpp -lpthread
./ec

KONTROLER:
g++ -Wall -std=c++11 controller.cpp -o controller -lmosquitto -ljsoncpp -lpthread
./controller

ENVIRONMENT
make
./environment


