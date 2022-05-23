# *HeSoC-mark* documentation

*If used for your research, please cite this work as:*
*Capodieci, N., Cavicchioli, R., Olmedo, I. S., Solieri, M., & Bertogna, M. (2020, August). Contending memory in heterogeneous SoCs: Evolution in NVIDIA Tegra embedded platforms. In 2020 IEEE 26th International Conference on Embedded and Real-Time Computing Systems and Applications (RTCSA) (pp. 1-10). IEEE.*

**HeSoC-mark** is an extensible, open-source collection of C++ applications aimed at stressing the  available  compute  engines  of  a  selected  platform  with memory intensive jobs. *HeSoC-mark* has been  developed  by  accounting  for  the  unprecedented  variety of  compute  engines  accessible  in  the  [NVIDIA Jetson Xavier development board](https://developer.nvidia.com/embedded/jetson-agx-xavier-developer-kit),  however, the  applications  contained  in *HeSoC-mark* are  designed  to  be independent modules, therefore they can be utilized in different heterogeneous systems. The constituent applications within *HeSoC-mark* are  able  to  generate  interference  and  memory-bound workloads from which we can measure both bandwidth and latencies.

License information can be found [here](LICENSE.md).  

*git clone* this repo and start experimenting with *HeSoC-mark* after reading this documentation

1. [Constituent benchmarks and applications](#constituent-benchmarks-and-applications)
2. [Test script generator: *interfgen*](#test-script-generator)
3. [Tested platforms](#tested-platforms)
4. [Known issues](#known-issues)

## Constituent benchmarks and applications

All the data processed by these applications is synthetic or randomly generated.

* **membench**: measures latencies in different points of the CPU complex cache hierarchy by performing memory read accesses over a pre-allocated 50 MiB buffer. Contained in this buffer, latencies are recorded from repeated pointer walks on a variable working-set size (WSS). In order to evaluate the CPU complex prefetching abilities and the memory controller ability to reorder memory request transactions, memory access pattern can be tuned to be sequential or random. Reported values are average latencies of reading a single integer word.

    * Compile: in *hesoc-mark/membench* folder type *make*
    * Usage and parameters: *./membench* for sequential memory access, or *./membench -r* for random memory access pattern.
.  
* **meminterf**: generates high-memory traffic by iteratively executing *memcpy* or *memset* POSIX-defined functions over buffers of parametrized dimension.

    * Compile: in *hesoc-mark/membench* folder type *make*
    * Usage and parameters: 
    *./meminterf [-h or --help] [-v or --verbose] [-s or --size=<size_t>] [-i or --iterations=<size_t>] [-t or --test=<memset|memcpy>]*  
    --help    Display help information
    --verbose   Self-explanatory. Default is false  
    --size=<size_t>     Size in MiB of each buffer. (There are two).  
    --test=<memset|memcpy>      Which test to run.  
    --iterations=<size_t>	 How many iterations for the test.  
.  
* **cudameasure**: submits a series of 100% memory-bound CUDA kernels. They perform copies from a source to a destination buffer. Kernels' execution time is profiled through [CUDA events](https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__EVENT.html).

    * Compile: in *hesoc-mark/cuda* folder type *make*
    * Usage and parameters: 
    *cudameasure [num of float per buffer] [iterations] (dryrun)*  
    (dryrun): type 'dryrun' as the last arg. to run an unmeasured kernel launch before the actual measure  
.  
 * **cudainterf**: generates interference from the GPU copy engines and Streaming Multiprocessors (SM), using NVIDIA CUDA as API. More specifically, the user can select to submit CUDA 100% memory bound kernels with/without UVM (Unified Virtual Memory) as the interfering SM activity. Available Copy Engine activities includes CUDA memset, memcpy (device to host, host to device and device to device) over buffers of parametrized dimension.
  
    * Compile: in *hesoc-mark/cuda* folder type *make*
    * Usage and parameters: 
    *./cudainterf [-h or --help] [-v or --verbose] [-s or --synch] [-d or --datasize=<size_t>] [-i or --iterations=<size_t>] [-m or --mode=<copyKernel|copyKernelUVM|d2d|memset|memcpy>]*  
    --help    Display help information  
    --verbose   Self-explanatory. Default is false"  
    --synch    Will call cudaStreamSynchronize once every two command submssions. Default is false.  
    --mode=<copyKernel|copyKernelUVM|d2d|memset|memcpy>   Which interference mode to run. Default is cudaMemcpy  
    --iterations=<size_t>	      How many iterations for the inteferring test.  
    --datasize=<size_t>    How many KILO float elements to use in the tests.  
.  

 * **oclinterf**: generates interference from the GPU copy engines and execution engine using the OpenCL API. More specifically, the user can select to submit an OpenCL 100% memory bound kernels as the interfering GPU activity. Available Copy Engine activities includes CL memory objects memset, memcpy (device to host, host to device and device to device) over buffers of parametrized dimension.
  
    * Compile: in *hesoc-mark/ocltest* folder type *make*
        * Makefile script must be changed according to the actual OpenCL installation folder!
    * Usage and parameters: 
     ./oclinterf [-h or --help] [-v or --verbose] [-s or --synch] [-d or --datasize=<size_t>] [-i or --iterations=<size_t>] 
    [-m or --mode=<copyKernel|copyKernelSVM|d2d|memset|memcpy>]
	[-l or --listplatforms] Will only list the available OpenCL platforms, so that you can later select your desidered one 
	[-p or --platformindexselect=\<int\>] Select the integer index of the desidered OpenCL platform. Default is 0 
    --help    Display help information
    --verbose   Self-explanatory. Default is false
    --synch    Will call synchronize to host once every two command submissions. Default is false.
    --mode=<copyKernel|copyKernelSVM|d2d|memset|memcpy>   Which interference mode to run. Default is clMemcpy
    --iterations=<size_t> How many iterations for the inteferring test
    --datasize=<size_t>    How many KILO float elements to use in the tests.
    
    _Do note: OpenCL SVM is not yet supported_
.
* **oclmeasure**: submits a series of 100% memory-bound OpenCL kernels. They perform copies from a source to a destination buffer. Kernels' execution time is profiled through [OpenCL events](https://www.khronos.org/files/opencl-1-2-quick-reference-card.pdf).

    * Compile: in *hesoc-mark/ocltest* folder type *make*
    * Usage and parameters: 
    *oclmeasure [num of float per buffer] [iterations] (platform ID) (dryrun)*  
    (platform ID): an integer representing the numerical ID of the CL platform to use. Optional. Default is 0.
    (dryrun): type 'dryrun' as the last arg. to run an unmeasured kernel launch before the actual measure  
.
* **vpipva**: submits computer-vision related algorithms through the NVIDIA Vision Programming Interface [VPI](https://docs.nvidia.com/vpi/index.html). Such algorithms can be executed on CPU, iGPU and the PVAs (Programmable Vision Accelerator, if available in the tested platform). 

    * Compile: in *hesoc-mark/vpipva* folder type *"cmake ."* then *make*
    * Usage and parameters: *./vpipva [iterations] [engine] [streams] [algorithms...]*
    iterations <size_t>	      How many iterations for the test.  
    engine [cpu|cuda|pva]    which engine to run the algorithms (cuda will use the iGPU).  
    streams <uint> how many concurrent streams to use in the test  
    algorithms <as many uint as the indicated streams, separated by a space> integers indicate the algorithm to run in the selected engine. More specifically:
        * 0: Stereo Disparity Estimator
        * 1: KLT Bounding Box Tracker
        * 2: Gaussian Pyramid Generator 
        * 3: Image Convolver
        * 4: Box Image Filter
        * 5: Separable Image Convolver
        * 6: Gaussian Image Filter
    
    Example: *./vpipva 3000 pva 8 6 6 6 6 6 6 6 6* will run 3000 iterations on 8 streams submitted to the PVA accelerator. Each stream will perform Gaussian Image Filter.
.  

* **trtdla**: creates Fully Connected Neural Networks and performs inference on a single instance of the DLA by exploiting the [TensorRT C++ API](https://developer.nvidia.com/tensorrt). This application can only run on Xavier-based platforms.
    * Compile: in *hesoc-mark/trtdla* folder type *make*
    * Usage and parameters:  
        * For building a network: *./testDLA [-h or --help] [-s or --size=<size_t>] [-l or --numlayers=<size_t>] [--useDLACore=<size_t>]*  
    --help          Display help information  
    --size=N  Specify the size of each layer the MLP fully connected network in terms of NxN.  
   --numlayers=N  Specify the number of layers of the network   
   --useDLACore=N  Specify a DLA engine for layers that support DLA. Value can range from 0 to n-1, where n is the number of DLA engines on the platform  
        * For loading and running a network: *./testDLA [-h or --help] [-p or --profile] [-f or --filein=</string/>] [-i or --iterations=<size_t>] [--useDLACore=<size_t>]*  
        --profile Activates the per-layer profile info
        --iterations=N specify how many iterations have to be performed
        --useDLACore=N  Specify a DLA engine for layers that support DLA. Value can range from 0 to n-1 where n is the number of DLA engines on the platform. 
.  
* **cpubench**: a  collection  of  single-core  CPU-only  workloads  adapted  from known    benchmark  suites.
    * Compile: in *hesoc-mark/cpubench* folder type *make*
    * Usage and parameters: *./cpubench [-h or --help] [-v or --verbose] [-d or --datacount=<size_t>] [-i or --iterations=<size_t>] [-t or --test=</string/>]*  
    --help    Display help information  
    --verbose Activate verbosity. Default false  
    --datacount=<size_t>  Specify the problem size/data count for the test. Default 4096  
    --iterations=<size_t>  Specify the number of iterations to be performed. Default 1  
    --test=</string/> Specify the benchmark. Possible values are MVT, BICG, GESUMMV and PATHFINDER. Default is MVT  
        * MVT, BICG, GESUMMV are re-elaborated versions of the CPU single-core implementations in the [PolyBench/GPU 1.0 suite](https://web.cse.ohio-state.edu/~pouchet.2/software/polybench/). 
        * PATHFINDER is a re-elaborated version of the CPU single-core implementation in the [RODINIA](http://rodinia.cs.virginia.edu/doku.php?id=pathfinder) suite.
        * LMS is a re-elaborated version of the CPU single-core implementation in the [TACLe Bench suite](https://github.com/tacle/tacle-bench/blob/master/bench/kernel/lms/lms.c). 

## Test script generator
In order to automate and test as much as possible, *hesoc-mark* includes the *interfgen* application. *interfgen* is  able to  generate  test  scripts  starting  from  an  XML  representation of  the  possible  set  of  tasks  for  which  we want to assess  the  impact of  memory  contention. 

To compile *interfgen*, move in *hesoc-mark/interfgen* and type *make*

Usage and parameters: 
*./interfgen [-h or --help] [-v or --verbose] [-f or --infile=</string/>.xml] [-o or --outdir=</string/>]*  
--help    Display help information  
--verbose Activate verbose parser and .sh script generation (for debug and test)  
--infile=</string/>.xml   Specify the input XML file for the test configuration.  
--outdir=</string/>     Specify the output directory to put the generated *.sh* scripts  

### XML schema for *interfgen*

XML nodes can be summarized as follows:

| Node        | Description           | Required  |
| ------------- |-------------| -----|
| test      | root node | yes. only one test node is allowed |
| id      | test id, integer      | yes. One ID per node is allowed  |
| iterations | num. of iteration per test | yes. |
| UT | Under Test Task | yes. Only one UT is allowed per test|
| IF | Interferring Task | yes. More than one IF is allowed |
| sleep_s | num. of seconds to sleep between the launch of the IFs and the launch of the UT  | no. Default 1 second |
| preprocstr   |  pre-process string for UTs. Can be used to set linux scheduling policies |   no |
| postprocstr   |  post-process string for UTs. Can be used to pipe-grepping and/or generic UT process output parsing |   no |

UT and IF are characterized by the following required attributes:

| Attribute     | Description     |
| ------------- |-------------|
| cmdLine      | command line string for the application |
| args | command line arguments for the application |
| core | specify the CPU-core in which to set the process affinity (will use [*taskset*](http://man7.org/linux/man-pages/man1/taskset.1.html))
| engine | A description of the application. Will be used to generate understandable filenames |

### *interfgen* example
	
Let us start from the following XML test description file, assuming a NVIDIA Jetson Xavier platform:
(*hesoc-mark/interfgen/testxml/CPUsTEST.xml*)

```xml
<?xml version="1.0" encoding="utf-8"?>
<test>

<id>
t1
</id>

<iterations>
1
</iterations>

<postprocstr>
| grep HESOCMARK >>  __THIS_FILENAME__
</postprocstr>

<preprocstr>
chrt -f 99
</preprocstr>

<UT cmdLine="../../membench/membench" args="" core="0" engine="CPU0"> </UT>
<IF cmdLine="../../membench/meminterf" args="-v --size=156 --iterations=855 --test=memcpy" core="1" engine="CPU1"></IF>
<IF cmdLine="../../membench/meminterf" args="-v --size=156 --iterations=855 --test=memcpy" core="2" engine="CPU2"></IF>
<IF cmdLine="../../membench/meminterf" args="-v --size=156 --iterations=855 --test=memcpy" core="3,4,5,6,7" engine="CPUs"></IF>
</test>
```

Do note the built-in reserved keywork **\_\_THIS_FILENAME\_\_**: *interfgen* will substitute this macro by assembling a *txt* output file whose name will be the same as the filename for the generated *sh* script followed by the test iteration number.

to generate scripts, let's move to *hesoc-mark/interfgen* and type:  
*./interfgen -f testxml/CPUsTEST.xml -o outscripts*
This will generate the following scripts in the *outscripts* directory:  

```
t1-CPU0_CPU1_CPU2_CPUs_CPUs_CPUs_CPUs_CPUs_.sh
t1-CPU0_CPU1_CPU2_.sh
t1-CPU0_CPU1_CPUs_CPUs_CPUs_CPUs_CPUs_.sh
t1-CPU0_CPU1_.sh
t1-CPU0_CPU2_CPUs_CPUs_CPUs_CPUs_CPUs_.sh*
t1-CPU0_CPU2_.sh
t1-CPU0_CPUs_CPUs_CPUs_CPUs_CPUs_.sh
```

As you can see, all the possible permutation of IF processes are considered for each generated *sh* file. Let us open *t1-CPU0_CPU1_.sh* to better understand what is going on.

```bash
#! /usr/bin/env bash

sysctl -w kernel.sched_rt_runtime_us=-1

for i in {1..1}
do
sudo taskset -c 1 ../../membench/meminterf -v --size=156 --iterations=855 --test=memcpy & PID_TO_KILL0=$!
sleep 1
sudo taskset -c 0 chrt -f 99 ../../membench/membench  | grep HESOCMARK >>   t1-CPU0_CPU1_.sh_$i.txt &  PID_TO_WAIT=$!
wait $PID_TO_WAIT

echo "done"
sudo kill -9 $PID_TO_KILL0
sudo killall -q meminterf

wait
done
```

First, we enable the system to schedule high priority tasks with no throttling.  
Then, for one iteration, we pin the *meminterf* app to CPU1, with specific parameters. Then we sleep for 1 second (a default value since we did not specify a delay between the UF and UT launches in the XML input file). Then we launch the task under test (on core 0, FIFO99 priority), which is *membench* in this case (with no params). There is a post processing string that parses *membench* output and stores it in a file.

Once the UT app is finished, all the interferring processes are then brutally killed.

Trivially, the scripts launches with *sudo ./t1-CPU0_CPU1_.sh*

More XML examples can be found in the *hesoc-mark/interfgen/testxml* folder.

## Tested platforms

As of today, the following boards have been tested for *HeSoC-mark*:

* NVIDIA Jetson Xavier
* NVIDIA Jetson TX2
* NVIDIA Jetson Nano

## Known issues

* Long running tasks pinned to a single core with a high priority (e.g. FIFO99) most likely will freeze the system.
* UF tasks runtime must be larger than the UT runtime. If an IF task finishes before the UT task, then interference measures will be under-estimated. The user must take care of these situations manually, e.g. setting a very large number of iterations for the IF tasks.
* *trtdla* attempts to run everything in the DLA. However, conversion operations before and just after each inference iteration will still run on the GPU. This will compromise latency measurements when a GPU task is submitted as the UT.









