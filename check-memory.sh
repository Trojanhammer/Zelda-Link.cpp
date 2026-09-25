#!/bin/bash
# Samples the running game's RAM usage (RSS - resident memory actually in use,
# not just reserved) once per second and logs it to memory-log.csv.
#
# Useful as a baseline now (no sprites/video loaded yet), then run again later
# once assets are in to see how much they actually add.
#
# Usage:
#   1. In one terminal: ./game
#   2. In another terminal: ./check-memory.sh
#   3. Play for a bit, then Ctrl+C (or just let it run until you quit the game)

PROCESS_NAME="game"
LOG_FILE="memory-log.csv"

PID=$(pgrep -x "$PROCESS_NAME" | head -n 1)

if [ -z "$PID" ]; then
    echo "No running process named '$PROCESS_NAME' found. Start the game first (./game), then run this script."
    exit 1
fi

echo "Watching PID $PID ($PROCESS_NAME). Logging to $LOG_FILE. Press Ctrl+C to stop."
echo "timestamp,rss_kb,rss_mb" > "$LOG_FILE"

while kill -0 "$PID" 2>/dev/null; do
    RSS_KB=$(ps -o rss= -p "$PID" 2>/dev/null | tr -d ' ')
    if [ -z "$RSS_KB" ]; then
        break
    fi
    RSS_MB=$(awk "BEGIN { printf \"%.2f\", $RSS_KB / 1024 }")
    TIMESTAMP=$(date +"%H:%M:%S")
    echo "$TIMESTAMP  ${RSS_MB} MB"
    echo "$TIMESTAMP,$RSS_KB,$RSS_MB" >> "$LOG_FILE"
    sleep 1
done

echo "Process ended (or stopped). Log saved to $LOG_FILE."
