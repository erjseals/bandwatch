//TODO: kind of messy. C and C++ syntax are mixed with no specific criteria

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "oclcommon.h"

#ifndef MAX_SOURCE_SIZE
#define MAX_SOURCE_SIZE (0x100000)
#endif

char str_temp[1024];
cl_platform_id platform_id;
cl_device_id device_id;
cl_uint num_devices;
cl_uint num_platforms;
cl_int errcode;
cl_context clGPUContext;
cl_kernel clKernel;
cl_command_queue clCommandQue;
cl_program clProgram;
cl_mem a_mem_obj;
cl_mem b_mem_obj;
FILE *fp;
char *source_str;
size_t source_size;


void read_cl_file()
{
	// Load the kernel source code into the array source_str
	fp = fopen("copykernel.cl", "r");
	if (!fp) {
		fprintf(stdout, "Failed to load kernel.\n");
		exit(1);
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	source_size = fread( source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose( fp );
}


void cl_initialization(const int32_t platform_index)
{

	// Get platform and device information
	errcode = clGetPlatformIDs(1, NULL, &num_platforms);
	if(errcode == CL_SUCCESS) printf("number of platforms is %d\n",num_platforms);
	else printf("Error getting platform IDs %i \n", errcode);

	cl_platform_id *plist = (cl_platform_id *) malloc(sizeof(cl_platform_id)*num_platforms);
	errcode = clGetPlatformIDs(num_platforms, plist, NULL);
	
	int i;
	for(i=0;i<num_platforms;i++){
		platform_id = plist[i];
		errcode = clGetPlatformInfo(platform_id,CL_PLATFORM_NAME, sizeof(str_temp), str_temp,NULL);
		if(errcode == CL_SUCCESS) printf("platform name is %s\n",str_temp);
		else printf("Error getting platform name\n");
	}

	if(platform_index > (num_platforms-1) ){
		std::cout << "Your selected platform does not exist in this OpenCL installation" << std::endl;
		exit(-1);
	}

	i = platform_index; 

	platform_id = plist[i];
	errcode = clGetPlatformInfo(platform_id, CL_PLATFORM_VERSION, sizeof(str_temp), str_temp,NULL);
	if(errcode == CL_SUCCESS) printf("platform version is %s\n",str_temp);
	else printf("Error getting platform version\n");

	clGetPlatformInfo(platform_id, CL_PLATFORM_VENDOR, sizeof(str_temp), str_temp, NULL);
	printf("Device info %s\n", str_temp);

	//TODO: fix this to allow for other devices...
	if(i!=2)
		errcode = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_devices);
	else errcode = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, &num_devices);

	if(errcode == CL_SUCCESS) printf("number of devices is %d\n", num_devices);
	else printf("Error getting device IDs\n");

	errcode = clGetDeviceInfo(device_id,CL_DEVICE_NAME, sizeof(str_temp), str_temp,NULL);
	if(errcode == CL_SUCCESS) printf("device name is %s\n",str_temp);
	else printf("Error getting device name\n");
	
	// Create an OpenCL context
	clGPUContext = clCreateContext( NULL, 1, &device_id, NULL, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating context\n");
 
	//Create a command-queue
	clCommandQue = clCreateCommandQueue(clGPUContext, device_id, CL_QUEUE_PROFILING_ENABLE, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating command queue\n");

	free(plist);

}

void cl_load_prog()
{
	// Create a program from the kernel source
	clProgram = clCreateProgramWithSource(clGPUContext, 1, (const char **)&source_str, (const size_t *)&source_size, &errcode);

	if(errcode != CL_SUCCESS) printf("Error in creating program\n");

	// Build the program
	errcode = clBuildProgram(clProgram, 1, &device_id, NULL, NULL, NULL);
	if(errcode != CL_SUCCESS) printf("Error in building program\n");
		
	// Create the OpenCL kernel
	clKernel = clCreateKernel(clProgram, "copykernel", &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating kernel\n");
	
	clFinish(clCommandQue);
}

void printHelp(){

    printf("Usage\n");
    printf("oclmeasure [num of float per buffer] [iterations] (platform_id) (dryrun)\n");
	printf("(platform_id): an integer representing the numerical ID of the CL platform to use. Optional. Default is 0.\n");
    printf("(dryrun): type 'dryrun' as the last arg. to run an unmeasured kernel launch before the actual measure\n");
}

void cl_clean_up()
{
	errcode = clFlush(clCommandQue);
	errcode = clFinish(clCommandQue);
	errcode = clReleaseKernel(clKernel);
	errcode = clReleaseProgram(clProgram);
	errcode = clReleaseMemObject(a_mem_obj);
	errcode = clReleaseMemObject(b_mem_obj);
	errcode = clReleaseCommandQueue(clCommandQue);
	errcode = clReleaseContext(clGPUContext);

	if(errcode != CL_SUCCESS) printf("Error in cleanup\n");
}


int main(int argc, char *argv[])
{

	bool dryrun = false;
	size_t elements = 2048;
	size_t iterations = 1;
	uint32_t platform_id = 0;

	 if(argc<3){

        printHelp();
        printf("Not enough arguments. Defaulting to no dry run.\n");
        printf("Num elements = %zu\n", elements);
        printf("Iterations = %zu\n", iterations);


    }else{

        elements = atoi(argv[1]);
        iterations = atoi(argv[2]);

		if(argc>3)
			platform_id = atoi(argv[3]);
        
        if(argc>4) 
            if(strcmp("dryrun",argv[4])==0)
                dryrun = true;

        printf("Executing with the following parameters: \n");
        printf("Num elements = %zu\n", elements);
        printf("Will perform a dry run? %s\n", (dryrun) ? "yes" : "no" );
    }

	printf("Will use platform at id %d\n", platform_id);
	fflush(stdout); 

	read_cl_file();
	cl_initialization(platform_id);
	cl_load_prog();

	const size_t datasize = sizeof(float) * elements;

	a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");
	b_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");

	float *A = (float*)malloc(datasize);
	memset(A,101,datasize);

	errcode = clEnqueueWriteBuffer(clCommandQue, a_mem_obj, CL_TRUE, 0, datasize, A, 0, NULL, NULL);
	if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation " << std::endl;

	clFlush(clCommandQue);
	clFinish(clCommandQue);

	size_t localWorkSize[1], globalWorkSize[1];
	localWorkSize[0] = 128; //TODO: find best
	globalWorkSize[0] = elements; //TODO: check for remainder

	errcode =  clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&b_mem_obj);
	errcode |= clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&a_mem_obj);
	if(errcode != CL_SUCCESS) printf("Error in setting arguments\n");

	
    if(dryrun){
		errcode = clEnqueueNDRangeKernel(clCommandQue, clKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if(errcode != CL_SUCCESS){ 
			printf("Error in launching kernel...\n");
			std::cout << getErrorString(errcode) << std::endl;
			exit(-1);
		}
		clFlush(clCommandQue);
		clFinish(clCommandQue);
        memset(A,101,datasize);
    }

	cl_event timing_evt;
	unsigned long start = 0;
	unsigned long end = 0;
	unsigned long elapsedTime;

	for(size_t i=0; i<iterations; i++){

		errcode = clEnqueueNDRangeKernel(clCommandQue, clKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, &timing_evt);

		clWaitForEvents(1, &timing_evt);

		clGetEventProfilingInfo(timing_evt,CL_PROFILING_COMMAND_START,
    							sizeof(cl_ulong),&start,NULL);       
		clGetEventProfilingInfo(timing_evt,CL_PROFILING_COMMAND_END,
    							sizeof(cl_ulong),&end,NULL);

		
		clReleaseEvent(timing_evt); // not sure it is necessary...

		elapsedTime = (end - start) / 1000;
        printf("Elapsed time is %zu us\n", elapsedTime);
        fflush(stdout);
    }

	if(errcode != CL_SUCCESS) printf("Error in launching kernel.... Benchmark invalid!\n");

	errcode = clEnqueueReadBuffer(clCommandQue, b_mem_obj, CL_TRUE, 0, datasize, A, 0, NULL, NULL);
	if(errcode != CL_SUCCESS) printf("Error in reading GPU mem\n");
    clFlush(clCommandQue);
	clFinish(clCommandQue);

	printf("CCHECK %f\n", A[0]); 
    fflush(stdout);

	free(A);
	cl_clean_up();
	return EXIT_SUCCESS;

}


