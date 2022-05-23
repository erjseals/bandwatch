# Memory Bandwidth Dynamic Regulation and Throttling for the Jetson Nano 

- Linux kernel for the following platform 
  - Jetson TX-1
- Experiment scripts to reproduce results from our paper
- Documentation
- See the slides here: https://docs.google.com/presentation/d/1GTzFajC9g4LfEN-imlE-JtDgsqb07BBJmNoPGDqJJnA/edit?usp=sharing

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
1. Obtain the source of the supported Linux kernel version for the platform (NVIDIA Developer, Jetson Download Center).

2. Flash the Jetson Nano following NVIDIA Developer instructions.

3. Make and load the kernel module on the platform.

## BandWatch

1. Build the kernel module.
```
cd bandwatch/ 
make
```

2. Insert the kernel module.
```
insmod bandwatch.ko
```

## Throttling

Use debugfs to specify throttling amount (0 - 31)

```
cd /sys/kernel/debug/bandwatch

AMOUNT = 10
echo $(AMOUNT) > throttle
```

