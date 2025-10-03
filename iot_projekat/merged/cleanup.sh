#!/bin/bash

echo "AGGRESSIVE CLEANUP - Killing ALL IoT processes..."

# KILL EVERYTHING MULTIPLE TIMES
for i in {1..3}; do
    echo "Round $i of cleanup..."
    
    # Kill by process name
    pkill -9 -f "./environment" 2>/dev/null
    pkill -9 -f "./controller" 2>/dev/null
    pkill -9 -f "./sensor_temp" 2>/dev/null
    pkill -9 -f "./heart_rate" 2>/dev/null
    pkill -9 -f "./machine_shutdown" 2>/dev/null
    pkill -9 -f "./emergency_call" 2>/dev/null
    pkill -9 -f "./app" 2>/dev/null
    
    # Kill by exact binary name
    killall environment 2>/dev/null
    killall controller 2>/dev/null
    killall sensor_temp 2>/dev/null
    killall heart_rate 2>/dev/null
    killall machine_shutdown 2>/dev/null
    killall emergency_call 2>/dev/null
    killall app 2>/dev/null
    
    sleep 1
done

# KILL PORT 8080 AGGRESSIVELY
echo "Killing everything on port 8080..."
for i in {1..5}; do
    PORT_PID=$(lsof -ti:8080 2>/dev/null)
    if [ ! -z "$PORT_PID" ]; then
        echo "Attempt $i: Killing PID $PORT_PID on port 8080"
        kill -9 $PORT_PID 2>/dev/null
        sleep 1
    else
        break
    fi
done

# DELETE ALL JSON FILES EVERYWHERE
echo "Deleting ALL JSON files..."
rm -f construction_site.json* 2>/dev/null
rm -f *.json 2>/dev/null
rm -f *.json.tmp 2>/dev/null
find . -name "construction_site*" -delete 2>/dev/null
find . -name "*.json.tmp" -delete 2>/dev/null

# RESTART MQTT BROKER
echo "Restarting MQTT broker..."
sudo systemctl restart mosquitto 2>/dev/null || sudo service mosquitto restart 2>/dev/null

sleep 2

echo " AGGRESSIVE CLEANUP COMPLETE!"

# FINAL STATUS CHECK
echo ""
echo "FINAL STATUS:"
echo "Port 8080: $(lsof -ti:8080 >/dev/null && echo 'STILL BUSY' || echo ' FREE')"
echo "Environment: $(pgrep -f './environment' >/dev/null && echo 'STILL RUNNING' || echo ' STOPPED')"
echo "All IoT processes: $(pgrep -f "./environment\|./controller\|./sensor" >/dev/null && echo ' STILL RUNNING' || echo ' ALL STOPPED')"
echo "JSON files: $(ls -la *.json 2>/dev/null | wc -l) found"
echo ""
echo " Ready for fresh start!"