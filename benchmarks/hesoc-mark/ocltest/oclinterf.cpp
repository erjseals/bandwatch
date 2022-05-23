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
#include <getopt.h>
#include <unistd.h>
#include <iostream>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

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
struct timespec start, end;

#define CL_MEMCPY 0
#define CL_MEMSET 1
#define CL_D2D 2
#define CL_C_KERNEL 3
#define CL_C_KERNEL_UVM 4

#define ITERATIONS_DEFAULT 101
#define MODE_DEFAULT CL_MEMCPY
#define KILO 1024
#define DATASIZE_DEFAULT 50

struct argsStruct {
	bool verbose;
    bool help;
    bool hasToSynch;
	bool listplatforms;
	int32_t mode;
    size_t datasize;
    size_t iterations;
	int32_t platform_index;
};

void cl_clean_up(void);

void memsets(const size_t datasize, const bool hasToSynch, const size_t iterations);
void memcpys(const size_t datasize, const bool hasToSynch, const size_t iterations);
void copykernel(const bool isUVM, const size_t datasize, const bool hasToSynch, const size_t iterations);
void d2d(const size_t datasize, const bool hasToSynch, const size_t iterations);

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


void cl_initialization(argsStruct &args)
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

	if(args.listplatforms) return; //that's it.

	if(args.platform_index > (num_platforms-1) ){
		std::cout << "Your selected platform does not exist in this OpenCL installation" << std::endl;
		exit(-1);
	}

	i = args.platform_index; 

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
	clCommandQue = clCreateCommandQueue(clGPUContext, device_id, 0, &errcode);
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

    std::cout << "Usage: ./oclinterf [-h or --help] [-v or --verbose] [-s or --synch] [-d or --datasize=<size_t>] [-i or --iterations=<size_t>] " << std::endl <<
    "[-m or --mode=<copyKernel|copyKernelSVM|d2d|memset|memcpy>]" << std::endl << 
	"[-l or --listplatforms] Will only list the available OpenCL platforms, so that you can later select your desidered one " << std::endl << 
	"[-p or --platformindexselect=<int>] Select the integer index of the desidered OpenCL platform. Default is 0 " << std::endl; 
    std::cout << "--help    Display help information" << std::endl;
    std::cout << "--verbose   Self-explanatory. Default is false" << std::endl;
    std::cout << "--synch    Will call synchronize to host once every two command submissions. Default is false." << std::endl;
    std::cout << "--mode=<copyKernel|copyKernelSVM|d2d|memset|memcpy>   Which interference mode to run. Default is clMemcpy" << std::endl;
    std::cout << "--iterations=<size_t>	      How many iterations for the inteferring test. Default is " << ITERATIONS_DEFAULT << std::endl;
    std::cout << "--datasize=<size_t>    How many KILO float elements to use in the tests. Default is " << DATASIZE_DEFAULT << " KILO elements" << std::endl;

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
			{"listplatforms", no_argument, 0, 'l'},
			{"plaformindexselect", required_argument, 0, 'p'},
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hlvsi:m:d:p:", long_options, &option_index);
	    if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true;
		case 'l': args.listplatforms = true;
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
		case 'p':
			if(optarg)
			{
				args.platform_index = atol(optarg);
			}
		break;
        case 'm':
            if(optarg){
                if(strcmp("memcpy",optarg)==0)
                    args.mode = CL_MEMCPY;
                else if (strcmp("memset",optarg)==0)
                    args.mode = CL_MEMSET;
                else if (strcmp("d2d",optarg)==0)
                    args.mode = CL_D2D;
                else if (strcmp("copyKernel",optarg)==0)
                    args.mode = CL_C_KERNEL;
                else if (strcmp("copyKernelSVM",optarg)==0)
                    args.mode = CL_C_KERNEL_UVM;
                else { std::cout << "Error in test specification" << std::endl; exit(-1); }
            }
            break;
        default: return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{

	size_t elements;

	argsStruct args;

    args.verbose = false;
    args.help = false;
    args.datasize = DATASIZE_DEFAULT;
    args.hasToSynch = false;
	args.listplatforms = false;
    args.mode = MODE_DEFAULT;
    args.iterations = ITERATIONS_DEFAULT;
	args.platform_index = 0;

    parseArgs(args,argc,argv);

    elements = KILO * args.datasize;

	 if(args.help)
        printHelp();

    if(args.verbose && !args.listplatforms){
        printf("Executing with the following parameters: \n");
        printf("Num elements = %zu\n", elements);
		printf("Will iterate for %zu times\n", args.iterations);
        if(args.mode==CL_C_KERNEL_UVM) printf("Mode is copyKernelSVM\n");
        else if(args.mode==CL_D2D) printf("Mode is opencl device to device copy\n");
        else if(args.mode==CL_MEMCPY) printf("Mode is clMemcpy\n");
        else if(args.mode==CL_MEMSET) printf("Mode is clMemset\n");
        else  printf("Mode is copyKernel with no SVM\n");
        printf("Will synch after each couple of commands? %s\n", (args.hasToSynch) ? "true" : "false" );
		printf("This program will use the OpenCL platform listed as element %d\n", args.platform_index);
        fflush(stdout); 
    }

	if(args.listplatforms){
		std::cout << "Will only list platforms..." << std::endl;
		cl_initialization(args);
		exit(EXIT_SUCCESS);
	}


	if(args.mode == CL_C_KERNEL_UVM || args.mode==CL_C_KERNEL)
		read_cl_file();

	cl_initialization(args);

	if(args.mode == CL_C_KERNEL_UVM || args.mode==CL_C_KERNEL)
		cl_load_prog();

	const size_t datasize = sizeof(float) * elements;
    const bool hasToSynch = args.hasToSynch;
    const size_t iterations = args.iterations;
	
    switch(args.mode){
        case CL_C_KERNEL_UVM:
            copykernel(true, datasize, hasToSynch, iterations);
        break;
        case CL_MEMSET:
            memsets(datasize, hasToSynch, iterations);
        break;
        case CL_MEMCPY:
            memcpys(datasize, hasToSynch, iterations);
        break;
        case CL_D2D:
            d2d(datasize, hasToSynch, iterations);
        break;
        default: 
            copykernel(false, datasize, hasToSynch, iterations);
    }


if(args.verbose)
	std::cout << "Done. Cleaning up now... " << std::endl;

cl_clean_up();
return EXIT_SUCCESS;

}

void d2d(const size_t datasize, const bool hasToSynch, const size_t iterations){

	a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");
	b_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");

	float *A = (float*)malloc(datasize);

	memset(A,101,datasize);

	errcode = clEnqueueWriteBuffer(clCommandQue, a_mem_obj, CL_TRUE, 0, datasize, A, 0, NULL, NULL);

	if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation " << std::endl;

	free(A);
	clFlush(clCommandQue);
	clFinish(clCommandQue);
    
    size_t i = 0;
    while(i<iterations){
        errcode = clEnqueueCopyBuffer(
				clCommandQue,
				a_mem_obj,
				b_mem_obj,
				0,
				0,
				datasize,
				0,
				NULL,
				NULL);

        errcode = clEnqueueCopyBuffer(
				clCommandQue,
				b_mem_obj,
				a_mem_obj,
				0,
				0,
				datasize,
				0,
				NULL,
				NULL);
        i++;
        if(hasToSynch) clFinish(clCommandQue);
    }

    if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation; Invalid benchmark! " << std::endl;


    clFlush(clCommandQue);
	clFinish(clCommandQue);
}

void memcpys(const size_t datasize, const bool hasToSynch, const size_t iterations){

    a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");
	b_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");

	float *A = (float*)malloc(datasize);
	float *B = (float*)malloc(datasize);

    memset(A,101,datasize);

    clFlush(clCommandQue);
	clFinish(clCommandQue);

    size_t i = 0;
    while(i<iterations){

		clEnqueueWriteBuffer(clCommandQue, a_mem_obj, CL_FALSE, 0, datasize, A, 0, NULL, NULL);
		errcode = clEnqueueReadBuffer(clCommandQue, b_mem_obj, CL_FALSE, 0, datasize, B, 0, NULL, NULL);
    
        i++;
        if(hasToSynch) clFinish(clCommandQue);
    }


       if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation; Invalid benchmark! " << std::endl;


	clFlush(clCommandQue);
	clFinish(clCommandQue);

    free(A);
	free(B);
}

void memsets(const size_t datasize, const bool hasToSynch, const size_t iterations){

	a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
	if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");

	float *A = (float*)malloc(datasize);

	  size_t i = 0;

	char c = 'c';

    while(i<iterations){
        errcode = clEnqueueFillBuffer (clCommandQue,
			a_mem_obj,
			&c,
			sizeof(char),
			0,
			datasize,
			0,
			NULL,
			NULL);
        i++;
        if(hasToSynch) clFinish(clCommandQue);
    }

        if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation; Invalid benchmark! " << std::endl;

	errcode = clEnqueueReadBuffer(clCommandQue, a_mem_obj, CL_TRUE, 0, datasize, A, 0, NULL, NULL);
	if(errcode != CL_SUCCESS) printf("Error in reading GPU mem\n");

	clFlush(clCommandQue);
	clFinish(clCommandQue);
	free(A);
}


void copykernel(const bool isUVM, const size_t datasize, const bool hasToSynch, const size_t iterations)
{

	size_t localWorkSize[1], globalWorkSize[1];
	localWorkSize[0] = 128; //TODO: find best
	globalWorkSize[0] = datasize/sizeof(float); //TODO: check for remainder

	float *A;
	float *B;

	if(!isUVM){
		a_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
		if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");
		b_mem_obj = clCreateBuffer(clGPUContext, CL_MEM_READ_WRITE, datasize, NULL, &errcode);
		if(errcode != CL_SUCCESS) printf("Error in creating the buffer\n");

		A = (float*)malloc(datasize);
		B = (float*)malloc(datasize);

    	memset(A,101,datasize);

		clEnqueueWriteBuffer(clCommandQue, a_mem_obj, CL_TRUE, 0, datasize, A, 0, NULL, NULL);

    	clFlush(clCommandQue);
		clFinish(clCommandQue);

		free(A);
		free(B);
	}
	else{
			//TODO: here we should query the driver to see the maximum supported SVM capability
			//and do stuff accordingly.
			//For devices that do not support fine grained SVM, explicit map/unmap operations
			//are necessary. Going to be quite messy.
			//Will be completed in the future.

			std::cout << "Not yet supported. Sorry" << std::endl;
			exit(EXIT_SUCCESS); 
	}

	errcode =  clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
	errcode |= clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
	if(errcode != CL_SUCCESS) printf("Error in setting arguments\n");

	for(size_t i=0; i<iterations; i++){
		clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&a_mem_obj);
		clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&b_mem_obj);
		errcode = clEnqueueNDRangeKernel(clCommandQue, clKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		clSetKernelArg(clKernel, 0, sizeof(cl_mem), (void *)&b_mem_obj);
		clSetKernelArg(clKernel, 1, sizeof(cl_mem), (void *)&a_mem_obj);
		errcode = clEnqueueNDRangeKernel(clCommandQue, clKernel, 1, NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);
		if(hasToSynch) clFinish(clCommandQue);
	}

	if(errcode!=CL_SUCCESS) std::cout << "Error in the last CL operation; Invalid benchmark! " << std::endl;

	clFlush(clCommandQue);
	clFinish(clCommandQue);
}

void cl_clean_up()
{
	errcode = clFlush(clCommandQue);
	errcode = clFinish(clCommandQue);
	errcode = clReleaseKernel(clKernel);
	errcode = clReleaseProgram(clProgram);

	if(a_mem_obj!=NULL)
		errcode = clReleaseMemObject(a_mem_obj);
	if(b_mem_obj!=NULL)
		errcode = clReleaseMemObject(b_mem_obj);

	errcode = clReleaseCommandQueue(clCommandQue);
	errcode = clReleaseContext(clGPUContext);
	if(errcode != CL_SUCCESS) printf("Error in cleanup\n");
}
