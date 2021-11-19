# Memory Bandwidth Throttling for the Jetson Nano 

- Linux kernel for the following platform 
  - Jetson TX-1
- Experiment scripts to reproduce results from our paper
- Documentation

# Pre-requisites
### Hardware
+ NVIDIA's Jetson TX-1 (or TX-2)

### Software
+ Linux for Tegra (Version 32.6.1) for Jetson TX-1
  + JetPack 4.6
  + Linux Kernel 4.9
  + Ubuntu 18.04
+ Git

# Step-by-step Instructions
1. Obtain the source of the supported Linux kernel version for the platform under test. Since this step is inherently platform / environment dependent, we leave it to the user to perform this step according to their platform of choice.

2. Patch the kernel source with the relevant RT-Gang patch from this repository.

3. Ensure that **CONFIG_SCHED_DEBUG** is enabled in the kernel configuration. Otherwise, the scheduling features won't be modifiable at runtime.

4. Build and install the kernel on the platform.

5. Once the system has rebooted, ensure that **RT_GANG_LOCK** is available as a scheduling feature in the file */sys/kernel/debug/sched_features*.

## Throttling

1. Build the kernel module.
```
cd throttling/kernel_module
make
```

2. Insert the kernel module.
```
insmod exe/bwlockmod.ko
```

3. Execute the RT-application and note its pid.

4. Execute the user-app and provide the necessary parameters as noted in Step-2.
```
# Sample RT-App: ./rt-work
# Sample Parameters: PID: 1296; Corun Threshold: 16348 (i.e., 100 MB/s in systems with 64B cache-word)
./rt-work &
[1] rt-work      1296

cd throttling/user_app
./test -p 1296 -v 1 -e 16348
```
