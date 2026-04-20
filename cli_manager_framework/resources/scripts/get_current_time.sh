#!/system/bin/sh

current_time=$(date '+%Y-%m-%d %H:%M:%S')

echo "$current_time"

echo "$current_time" > current_time.txt

if [ $? -eq 0 ]; then
    echo "Current time has been written to current_time.txt"
    exit 0
else
    echo "Failed to write to current_time.txt" >&2
    exit 1
fi
