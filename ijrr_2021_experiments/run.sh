

# 16 GB
ulimit -v $((16 * 1024 * 1024)) # kilobytes

# 10m
ulimit -t $((10 * 60)) # seconds

# Display?
ulimit -a

python3 run.py
