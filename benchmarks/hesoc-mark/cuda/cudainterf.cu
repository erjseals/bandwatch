#include <cuda_runtime.h>
#include <cuda_profiler_api.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <sys/time.h>

#define LOG 1

#define CUDA_MEMCPY 0
#define CUDA_MEMSET 1
#define CUDA_D2D 2
#define CUDA_C_KERNEL 3
#define CUDA_C_KERNEL_UVM 4

#define ITERATIONS_DEFAULT 101
#define MODE_DEFAULT CUDA_MEMCPY
#define KILO 1024
#define DATASIZE_DEFAULT 50

void memsets(const size_t datasize, const bool hasToSynch, const size_t iterations);
void memcpys(const size_t datasize, const bool hasToSynch, const size_t iterations);
void copykernel(const bool isUVM, const size_t datasize, const bool hasToSynch, const size_t iterations);
void d2d(const size_t datasize, const bool hasToSynch, const size_t iterations);

volatile size_t iter = 0;
volatile unsigned int g_start = 0;

struct argsStruct {
	bool verbose;
    bool help;
    bool hasToSynch;
	int32_t mode;
    size_t datasize;
    size_t iterations;
};

#if LOG
unsigned int get_usecs()
{
  struct timeval time;
  gettimeofday(&time, NULL);
  return (time.tv_sec * 1000000 + time.tv_usec);
}

void start(){
  iter = 0;
  g_start = get_usecs();
}
void finish(){
  float dur = get_usecs() - g_start; 
  float dur_in_sec = (float)dur / 1000000; 
  printf("Total iterations: %ld\n",iter);
  printf("elapsed = %.2f sec ( %.0f usec )\n", dur_in_sec, dur);
  float bw = (float)iter * 102400 * 1024 * sizeof(float) / dur_in_sec / 1024 / 1024;
  printf("Memcpy BW = %.2f MB/s\n", bw*2);
  printf("Memset BW = %.2f MB/s\n", bw);
  iter = 10000000;
}

void signal_handler(int sigNo)
{
  switch(sigNo) {
  case SIGUSR1: {
    start();
    break;
  }
  case SIGUSR2: {
    finish();
    break;
  }
  default:
    break;
  }
}
#endif


//no boundary checks to avoid unnecessary "if"s.
__global__ void copyKernelGPU(float *a, float *b){

    const uint32_t gid = threadIdx.x + blockIdx.x * blockDim.x; 
    b[gid] = a[gid];
}

void printHelp(){

    std::cout << "Usage: ./cudainterf [-h or --help] [-v or --verbose] [-s or --synch] [-d or --datasize=<size_t>] [-i or --iterations=<size_t>] " << std::endl <<
    "[-m or --mode=<copyKernel|copyKernelUVM|d2d|memset|memcpy>]" << std::endl;
    std::cout << "--help    Display help information" << std::endl;
    std::cout << "--verbose   Self-explanatory. Default is false" << std::endl;
    std::cout << "--synch    Will call cudaStreamSynchronize once every two command submssions. Default is false." << std::endl;
    std::cout << "--mode=<copyKernel|copyKernelUVM|d2d|memset|memcpy>   Which interference mode to run. Default is cudaMemcpy" << std::endl;
    std::cout << "--iterations=<size_t>	      How many iterations for the innteferring test. Default is " << ITERATIONS_DEFAULT << std::endl;
    std::cout << "--datasize=<size_t>    How many KILO float elements to use in the tests. Default is " << DATASIZE_DEFAULT << " KILO eleements" << std::endl;

    exit(EXIT_SUCCESS);

}

bool parseArgs(argsStruct &args, int argc, char* argv[])
{
    while (1)
    {
        int arg;
        static struct option long_options[] = {{"help", no_argument, 0, 'h'},
            {"datasize", required_argument, 0, 'd'},
            {"verbose", no_argument, 0, 'v'},
            {"synch", no_argument, 0, 's'},
            {"iterations", required_argument, 0, 'i'},
            {"mode", required_argument, 0, 'm'},
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hvsi:m:d:", long_options, &option_index);
	    if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true;
	    case 'v': args.verbose = true; break;
        case 's': args.hasToSynch = true; break;
        case 'i':
            if (optarg)
            {
                args.iterations = atol(optarg);
            }
            break;
        case 'd':
        if (optarg)
            {
                args.datasize = atol(optarg);
            }
        break;
        case 'm':
            if(optarg){
                if(strcmp("memcpy",optarg)==0)
                    args.mode = CUDA_MEMCPY;
                else if (strcmp("memset",optarg)==0)
                    args.mode = CUDA_MEMSET;
                else if (strcmp("d2d",optarg)==0)
                    args.mode = CUDA_D2D;
                else if (strcmp("copyKernel",optarg)==0)
                    args.mode = CUDA_C_KERNEL;
                else if (strcmp("copyKernelUVM",optarg)==0)
                    args.mode = CUDA_C_KERNEL_UVM;
                else { std::cout << "Error in test specification" << std::endl; exit(-1); }
            }
            break;
        default: return false;
        }
    }
    return true;
}


int main(int argc, char *argv[]){

    size_t elements;

    argsStruct args;
    args.verbose = false;
    args.help = false;
    args.datasize = DATASIZE_DEFAULT;
    args.hasToSynch = false;
    args.mode = MODE_DEFAULT;
    args.iterations = ITERATIONS_DEFAULT;

    if(argc<=1)
      args.verbose = true;

    parseArgs(args,argc,argv);

    elements = KILO * args.datasize;

    if(args.help)
        printHelp();

    if(args.verbose){
        printf("Executing with the following parameters: \n");
        printf("Num elements = %zu\n", elements);
	printf("Will iterate for %zu times\n", args.iterations);
        if(args.mode==CUDA_C_KERNEL_UVM) printf("Mode is copyKernelUVM\n");
        else if(args.mode==CUDA_D2D) printf("Mode is cuda device to device copy\n");
        else if(args.mode==CUDA_MEMCPY) printf("Mode is cudaMemcpy\n");
        else if(args.mode==CUDA_MEMSET) printf("Mode is cudaMemset\n");
        else  printf("Mode is copyKernel with no UVM\n");
        printf("Will synch after each couple of commands? %s\n", (args.hasToSynch) ? "true" : "false" );
        fflush(stdout); 
    }

#if LOG
    /* set signals to terminate once time has been reached */
    if (signal(SIGUSR1,signal_handler) == SIG_ERR)
      printf("Failed to setup SIGUSR1\n");
    if (signal(SIGUSR2,signal_handler) == SIG_ERR)
      printf("Failed to setup SIGUSR2\n");
#endif

    const size_t datasize = sizeof(float) * elements;
    const bool hasToSynch = args.hasToSynch;
    const size_t iterations = args.iterations;

    switch(args.mode){
        case CUDA_C_KERNEL_UVM:
            copykernel(true, datasize, hasToSynch, iterations);
        break;
        case CUDA_MEMSET:
            memsets(datasize, hasToSynch, iterations);
        break;
        case CUDA_MEMCPY:
            memcpys(datasize, hasToSynch, iterations);
        break;
        case CUDA_D2D:
            d2d(datasize, hasToSynch, iterations);
        break;
        default: 
            copykernel(false, datasize, hasToSynch, iterations);
    }

    if(args.verbose)
        std::cout << argv[0] << ": Done" << std::endl;

    return EXIT_SUCCESS;
}

void copykernel(const bool isUVM, const size_t datasize, const bool hasToSynch, const size_t iterations){

    cudaStream_t s;
    cudaStreamCreate(&s);

    float *hData; 
    float *dData0; 
    float *dData1;

    if(!isUVM){
        cudaMallocHost((void**)&hData, datasize);
        cudaMalloc((void**)&dData0, datasize);
        cudaMalloc((void**)&dData1, datasize); 
        memset(hData,101,datasize);
        cudaMemcpyAsync(dData0, hData, datasize, cudaMemcpyHostToDevice, s);
        cudaStreamSynchronize(s);
    }else{
        cudaMallocManaged((void**)&dData0,datasize);
        cudaMallocManaged((void**)&dData1,datasize);
        memset(dData0,101,datasize);
    }

    const uint32_t threads = 128; //TODO: find best.
    const uint32_t blocks = datasize/sizeof(float)/threads; //TODO: check for remainder

    size_t i = 0;
    while(i<iterations){
        copyKernelGPU<<<blocks,threads,0,s>>>(dData0,dData1);
        copyKernelGPU<<<blocks,threads,0,s>>>(dData1,dData0);
        i++;
        if(hasToSynch) cudaStreamSynchronize(s);
    }

    cudaFree(dData0);
    cudaFree(dData1);
    if(!isUVM)
        cudaFreeHost(hData);

}

void d2d(const size_t datasize, const bool hasToSynch, const size_t iterations){

    cudaStream_t s;
    cudaStreamCreate(&s);
    float *hData; cudaMallocHost((void**)&hData, datasize);
    float *dData0; cudaMalloc((void**)&dData0, datasize);
    float *dData1; cudaMalloc((void**)&dData1, datasize); 

    memset(hData,101,datasize);
    cudaMemcpyAsync(dData0, hData, datasize, cudaMemcpyHostToDevice, s);
    cudaStreamSynchronize(s);
    cudaFreeHost(hData);
    
    size_t i = 0;
    while(i<iterations){
        cudaMemcpyAsync(dData0,dData1,datasize, cudaMemcpyDeviceToDevice, s);
        cudaMemcpyAsync(dData1,dData0,datasize, cudaMemcpyDeviceToDevice, s);
        i++;
        if(hasToSynch) cudaStreamSynchronize(s);
    }

    cudaFree(dData0);
    cudaFree(dData1);
}

void memcpys(const size_t datasize, const bool hasToSynch, const size_t iterations){

    cudaStream_t s;
    cudaStreamCreate(&s);
    float *hData; cudaMallocHost((void**)&hData, datasize);
    float *dData; cudaMalloc((void**)&dData, datasize);

    memset(hData,101,datasize);
    cudaStreamSynchronize(s);

    iter = 0;
    while(iter<iterations){
        cudaMemcpyAsync(dData, hData, datasize, cudaMemcpyHostToDevice, s);
	      cudaMemcpyAsync(hData, dData, datasize, cudaMemcpyDeviceToHost, s);
        iter++;
        if(hasToSynch) cudaStreamSynchronize(s);
    }

    cudaFreeHost(hData);
    cudaFree(dData);
}

void memsets(const size_t datasize, const bool hasToSynch, const size_t iterations){

    cudaStream_t s;
    cudaStreamCreate(&s);

    float *hData; cudaMallocHost((void**)&hData, sizeof(float)); 
    float *dData; cudaMalloc((void**)&dData, datasize);
	
    cudaStreamSynchronize(s);

    iter = 0;
    while(iter<iterations){
        cudaMemset((void**)dData, 'c', datasize);	
        iter++;
        if(hasToSynch) cudaStreamSynchronize(s);
    }

    cudaMemcpyAsync(hData,dData,sizeof(float),cudaMemcpyDeviceToHost, s);
    cudaStreamSynchronize(s);
    
    cudaFreeHost(hData);
    cudaFree(dData);
}
