CXX = g++
CXXFLAGS = -Wall -std=c++11 -Iinclude
LIBS = -lmosquitto -ljsoncpp -lpthread

# Lista izvora i binarnih fajlova
SRCS = controller.cpp \
       sensor_temp.cpp \
       machine_shutdown_relay.cpp \
       emergency_call_module.cpp \
       environment.cpp \
       heart_rate.cpp \
       app.cpp

BINS = $(SRCS:.cpp=)

all: $(BINS)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@ $(LIBS)

# Run target koji pokreće sve binarne fajlove paralelno
run: $(BINS)
	./environment &          # prvo HTTP server
	sleep 1                  # sačekaj da se server digne
	./app &
	./controller &
	./sensor_temp &
	./heart_rate &
	./machine_shutdown_relay &
	./emergency_call_module &
	wait

clean:
	rm -f $(BINS)