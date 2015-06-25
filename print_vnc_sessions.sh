ps aux | grep vnc | awk '{print $12;}' | grep : | sort
