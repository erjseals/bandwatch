#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>

#define MEGA (1024*1024)

#define MEMCPY_TEST 0
#define MEMSET_TEST 1

#define DEFAULT_SIZE (50)
#define DEFAULT_ITERATIONS 10
#define DEFAULT_TESTID (MEMCPY_TEST)

struct argStruct {
size_t size;
uint8_t testID;
size_t iterations;
bool verbose;
bool help;
};

inline bool parseArgs(argStruct &args, int argc, char* argv[])
{
    while (1)
    {
        int arg;
        static struct option long_options[] = {{"help", no_argument, 0, 'h'},
            {"size", required_argument, 0, 's'},
            {"verbose", no_argument, 0, 'v'},
            {"iterations", required_argument, 0, 'i'},
            {"test", required_argument, 0, 't'},
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hvs:i:t:", long_options, &option_index);
	    if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true;
	case 'v': args.verbose = true; break;
        case 's':
            if (optarg)
            {
                args.size = atol(optarg);
                if(args.size==0) { std::cout << "Invalid size" << std::endl; exit(-1); }
            }
            break;
        case 'i':
            if (optarg)
            {
                args.iterations = atol(optarg);
            }
            break;
        case 't':
            if(optarg){
                if(strcmp("memcpy",optarg)==0||strcmp("MEMCPY",optarg)==0)
                    args.testID = MEMCPY_TEST;
                else if (strcmp("memset",optarg)==0||strcmp("MEMSET",optarg)==0)
                    args.testID = MEMSET_TEST;
                else { std::cout << "Error in test specification" << std::endl; exit(-1); }
            }
            break;
        default: return false;
        }
    }
    return true;
}


void showHelp(){

    std::cout << "Usage: ./meminterf [-h or --help] [-v or --verbose] [-s or --size=<size_t>] [-i or --iterations=<size_t>] [-t or --test=<memset|memcpy>]" << std::endl;
    std::cout << "--help    Display help information" << std::endl;
    std::cout << "--verbose   Self-explanatory. Default is false" << std::endl;
    std::cout << "--size=<size_t>     Size in MiB of each buffer. (There are two). Default is " << DEFAULT_SIZE << " MiB" << std::endl;
    std::cout << "--test=<memset|memcpy>      Which test to run. Default is " <<  ((DEFAULT_TESTID==MEMCPY_TEST) ? "memcpy" : "memset") << std::endl;
    std::cout << "--iterations=<size_t>	      How many iterations for the test. Default is " << DEFAULT_ITERATIONS << std::endl;

    exit(EXIT_SUCCESS);
}

void memcpy_test(int8_t *A, int8_t *B, const size_t iters, const size_t size){

    for(size_t i=0; i<iters; i++){
        memcpy(A,B,size);
        memcpy(B,A,size);
    }

}

//TODO FIX
void memset_test(int8_t *A, int8_t *B, const size_t iters, const size_t size){
    
    for(size_t i=0; i<iters; i++){
        memset(A,B[i],size);
        memset(B,A[i],size);
    }

}

int main(int argc, char *argv[]){

    argStruct args;
    args.size = DEFAULT_SIZE;
    args.iterations = DEFAULT_ITERATIONS;
    args.help = false;
    args.testID = DEFAULT_TESTID;
    args.verbose = false;

    parseArgs(args,argc,argv);

    if(args.help)
        showHelp();

    args.size = args.size * MEGA;

   if(args.verbose){
   	std::cout << "Test will iterate " << args.iterations << " Times on two buffers of size " << args.size << " B" << std::endl;
   }

    int8_t *A = (int8_t*) malloc(args.size);
    int8_t *B = (int8_t*) malloc(args.size);

    memset(A,'a',args.size);
    memset(B,'b',args.size);

    if(args.testID==MEMCPY_TEST){
	if(args.verbose) std::cout << "Will execute memcpy test" << std::endl; 
        memcpy_test(A,B,args.iterations,args.size);
    }
    else if(args.testID==MEMSET_TEST){
	if(args.verbose) std::cout << "Will execute memset test" << std::endl;
        memset_test(A,B,args.iterations,args.size);
    }

    if(args.verbose)
	    std::cout << argv[0] << ": Done" << std::endl;

    free(A);
    free(B);

    return EXIT_SUCCESS;
}
