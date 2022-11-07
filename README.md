# Bandwatch 

Eric Seals (ericseals@ku.edu)

Heechul Yun (heechul@illinois.edu)

## Preparation

Recommended kernel settings are as follows:

	CONFIG_ACPI_PROCESSOR=n
	CONFIG_CPU_IDLE=n

## Build and Install

On a Jetson product in the bandwatch directory, first compile the code:
```shell
$ make
```
Load the kernel mode and specify Tegra X1 hardware memory counter:
```shell
$ insmod memguard.ko g_hw_counter_id=0x17
```

## Usage

Once the module is loaded, Bandwatch can be used for dynamic memory regulation in an effort to maintain RT CPU core performance against co-running CPU and GPU applications. The current configuration will protect CPU core 0 against interference.

A note on memory controller utilization on Tegra. The MC runs at a clock speed of 1.6GHz so at a 10us sample period, 10% utilization is measured as 1600 ticks by the activity monitor.

## Individual Components

The Memory Activity Monitor and GPU throttling mechanism can be tested individually by exploring the individual_modules directory.

### GPU Throttle

On a Jetson product in the `/bandwatch/individual_modules/throttle` directory, first compile the code:

```shell
$ make
$ insmod throttle.ko
```
The throttling level is controlled using the debugfs interface. On Tegra X1 architectures, the memory throttling has the granularity of 32 levels. For example, the following sets the level to 8: 
```shell
$ echo 8 > /sys/kernel/debug/throttle/throttle
```

### Activity Monitor
On a Jetson product in the `/bandwatch/individual_modules/actmon` directory, first compile the code:

```shell
$ make
$ insmod actmon.ko
```
You can run the simple script `logACTMON.sh` to output to the console, every 100ms, the total average count of memory cycles within the memory controller. 