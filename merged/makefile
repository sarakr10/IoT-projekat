CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread
LIBS = -lmosquitto -ljsoncpp

# Izvršni fajlovi
TARGETS = environment controller heart_rate temperature emergency shutdown

all: $(TARGETS)

environment: environment.cpp
	$(CXX) $(CXXFLAGS) -o environment environment.cpp $(LIBS)

controller: upravljac/controller_merged.cpp
	$(CXX) $(CXXFLAGS) -o controller upravljac/controller_merged.cpp $(LIBS)

heart_rate: heart_rate_sensor/heart_rate.cpp
	$(CXX) $(CXXFLAGS) -o heart_rate heart_rate_sensor/heart_rate.cpp $(LIBS)

temperature: temperature_sensor/sensor_temp.cpp
	$(CXX) $(CXXFLAGS) -o temperature temperature_sensor/sensor_temp.cpp $(LIBS)

emergency: emergency_call/emergency_call_merged.cpp
	$(CXX) $(CXXFLAGS) -o emergency emergency_call/emergency_call_merged.cpp $(LIBS)

shutdown: machine_shutdown/machine_shutdown_merged.cpp
	$(CXX) $(CXXFLAGS) -o shutdown machine_shutdown/machine_shutdown_merged.cpp $(LIBS)

clean:
	rm -f $(TARGETS)
