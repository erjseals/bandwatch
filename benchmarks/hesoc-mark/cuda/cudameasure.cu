#include <cuda_runtime.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <unistd.h>

//no boundary checks to avoid unnecessary "if"s.
__global__ void copyKernelGPU(float *a, float *b){

    const uint32_t gid = threadIdx.x + blockIdx.x * blockDim.x; 
    b[gid] = a[gid];
}

void printHelp(){
    printf("Usage\n");
    printf("cudameasure [num of float per buffer] [iterations] (dryrun)\n");
    printf("(dryrun): type 'dryrun' as the last arg. to run an unmeasured kernel launch before the actual measure\n");
}

int main(int argc, char *argv[]){

    bool dryrun = false;
    size_t elements = 2048;
    size_t iterations = 1;

    if(argc<3){

        printHelp();
        printf("Not enough arguments. Defaulting to no dry run.\n");
        printf("Num elements = %zu\n", elements);
        printf("Iterations = %zu\n", iterations);

    }else{

        elements = atoi(argv[1]);
        iterations = atoi(argv[2]);
        
        if(argc>3) 
            if(strcmp("dryrun",argv[3])==0)
                dryrun = true;

        printf("Executing with the following parameters: \n");
        printf("Num elements = %zu\n", elements);
        printf("Will perform a dry run? %s\n", (dryrun) ? "yes" : "no" );

    }

    fflush(stdout); 

    const size_t datasize = sizeof(float) * elements;

    cudaStream_t s;
    cudaStreamCreate(&s);
    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    float elapsedTime;

    float *hData; 
    float *dData0; 
    float *dData1;

    cudaMallocHost((void**)&hData, datasize);
    cudaMalloc((void**)&dData0, datasize);
    cudaMalloc((void**)&dData1, datasize); 
    memset(hData,101,datasize);
    cudaMemcpyAsync(dData0, hData, datasize, cudaMemcpyHostToDevice, s);
    cudaStreamSynchronize(s);

    const uint32_t threads = 128; //TODO: find best.
    const uint32_t blocks = datasize/sizeof(float)/threads; //TODO: check for remainder

    if(dryrun){
        copyKernelGPU<<<blocks,threads,0,s>>>(dData0,dData1);
        cudaMemcpyAsync(hData,dData1,datasize,cudaMemcpyHostToDevice,s);
        cudaStreamSynchronize(s);
        memset(hData,101,datasize);
    }

    usleep(10000);

    for(size_t i=0; i<iterations; i++){
        cudaEventRecord(start, s);
        copyKernelGPU<<<blocks,threads,0,s>>>(dData0,dData1);
        cudaStreamSynchronize(s);
        cudaEventRecord(stop, s);
        cudaEventSynchronize(stop);
        cudaEventElapsedTime(&elapsedTime, start, stop);
        printf("%f\n", elapsedTime);
        fflush(stdout);
    }

    cudaMemcpyAsync(hData,dData1,datasize,cudaMemcpyHostToDevice,s); 
    cudaStreamSynchronize(s);

    printf("CCHECK %f\n", hData[0]); 
    fflush(stdout);

    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    cudaFree(dData0);
    cudaFree(dData1);
    cudaFreeHost(hData);
    cudaStreamDestroy(s);

    return EXIT_SUCCESS;
}
