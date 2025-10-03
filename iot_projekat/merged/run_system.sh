#!/bin/bash

# Pokreni Mosquitto broker u pozadini
echo "[1/6] Pokrećem Mosquitto broker..."
mosquitto -d
sleep 2

# Pokreni environment simulator
echo "[2/6] Pokrećem environment simulator..."
gnome-terminal --title="ENVIRONMENT" -- bash -c "./environment; exec bash"
sleep 1

# Pokreni controller
echo "[3/6] Pokrećem controller..."
gnome-terminal --title="CONTROLLER" -- bash -c "./controller; exec bash"
sleep 1

# Pokreni heart rate sensor
echo "[4/6] Pokrećem heart_rate sensor..."
gnome-terminal --title="HEART_RATE_SENSOR" -- bash -c "./heart_rate; exec bash"
sleep 1

# Pokreni temperature sensor
echo "[5/6] Pokrećem temperature sensor..."
gnome-terminal --title="TEMPERATURE_SENSOR" -- bash -c "./temperature; exec bash"
sleep 1

# Pokreni actuators
echo "[6/6] Pokrećem actuators (emergency i shutdown)..."
gnome-terminal --title="EMERGENCY_ACTUATOR" -- bash -c "./emergency; exec bash"
gnome-terminal --title="SHUTDOWN_ACTUATOR" -- bash -c "./shutdown; exec bash"
sleep 1

echo "✅ Sistem je pokrenut! Svaka komponenta ima svoj terminal sa naslovom."
