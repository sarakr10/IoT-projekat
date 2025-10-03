CXX = g++
CXXFLAGS = -Wall -std=c++11 -Iinclude
LIBS = -lmosquitto -ljsoncpp -lpthread

SRCS = controller.cpp \
       sensor_temp.cpp \
       machine_shutdown.cpp \
       emergency_call.cpp \
       environment.cpp \
       heart_rate.cpp \
       app.cpp

BINS = $(SRCS:.cpp=)

all: $(BINS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

run: $(BINS)
	@echo " Starting IoT system with signal handling..."
	@echo " Use Ctrl+C to stop gracefully"
	@trap 'echo ""; echo " Stopping system..."; ./cleanup.sh; exit 0' INT TERM
	./environment &
	sleep 4
	./controller &
	sleep 2
	./sensor_temp &
	./heart_rate &
	./machine_shutdown &
	./emergency_call &
	sleep 1
	./app &
	wait

run-basic: $(BINS)
	@echo " Starting basic IoT system with signal handling..."
	@echo " Use Ctrl+C to stop gracefully"
	@trap 'echo ""; echo " Stopping system..."; ./cleanup.sh; exit 0' INT TERM
	./environment &
	sleep 4
	./controller &
	sleep 2
	./sensor_temp &
	./heart_rate &
	./machine_shutdown &
	./emergency_call &
	wait

clean:
	rm -f $(BINS)
	rm -f *.json *.json.tmp 2>/dev/null || true

clean-all:
	@echo "🧹 Full system cleanup..."
	./cleanup.sh
	rm -f $(BINS)

status:
	@echo "📊 System Status:"
	@echo "MQTT broker: $(pgrep mosquitto >/dev/null && echo ' Running' || echo ' Not running')"
	@echo "Port 8080: $(lsof -ti:8080 >/dev/null && echo 'Busy' || echo ' Free')"
	@echo "Environment: $(pgrep -f './environment' >/dev/null && echo ' Running' || echo ' Stopped')"
	@echo "Controller: $(pgrep -f './controller' >/dev/null && echo ' Running' || echo ' Stopped')"
	@echo "Sensors: $(pgrep -f './sensor' >/dev/null && echo ' Running' || echo ' Stopped')"
	@echo "JSON files: $(ls -1 *.json 2>/dev/null | wc -l) found"

.PHONY: all run run-basic clean clean-all status