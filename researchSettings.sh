sudo nvpmodel -m 0
sudo jetson_clocks

# Make sure trace file is large enough
sudo echo 4096 > /sys/kernel/debug/tracing/buffer_size_kb

# Disable GUI
# sudo systemctl set-default multi-user.target

# Enable GUI
sudo systemctl set-default graphical.target
