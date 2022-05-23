/**
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <getopt.h>

#include "../common/common.h"

#define DATA_TYPE float

#ifndef M_PI
#define M_PI 3.14159
#endif

#define ALPHA 32412.0f
#define BETA 2123.0f
#define M_SEED 9

#define MIN(a, b) ((a)<=(b) ? (a) : (b))

#define SAMPLING 5
double lms_sintab[ SAMPLING + 1 ] = {
  0.00000000000000000,
  0.43701603620715901,
  0.83125389555938600,
  1.14412282743652560,
  1.34499703920997637,
  1.41421356237309381,
};


#define MVT 0
#define BICG 1
#define GESUMMV 2
#define PATHFINDER 3
#define LMS 4

struct ccpplexargs {
	bool verbosity;
	bool help;
	int N;
    size_t iterations;
    int test;
	size_t datasize;
} args;

double rtclock()
{
    struct timezone Tzp;
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, &Tzp);
    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
}

DATA_TYPE lms_sinus( int i )
{
  int s = i % ( 4 * SAMPLING );
  if ( s >= ( 2 * SAMPLING ) )
    return -lms_sintab[ ( s > 3 * SAMPLING ) ?
                                            ( 4 * SAMPLING - s ) : ( s - 2 * SAMPLING ) ];
  return DATA_TYPE(lms_sintab[ ( s > SAMPLING ) ? ( 2 * SAMPLING - s ) : s ]);
}

void lms_init(DATA_TYPE  *lms_input, DATA_TYPE *b, DATA_TYPE *history)
{
  unsigned long seed = 1;
  int k;
  const int N = args.N;
  const int L = N/10;

  for (int i = 0; i <= L; i++ ) {
    b[i] = 0.0;
    history[i] = 0.0;
  }

  lms_input[ 0 ] = 0.0;
  {
    double v1, v2, r;
    const double scaleFactor = 0.000000000931322574615478515625;
    do {
      // generate two random numbers between -1.0 and +1.0
      seed = seed * 1103515245 + 12345;
      v1 = ( seed & 0x00007fffffff ) * scaleFactor - 1.0;
      seed = seed * 1103515245 + 12345;
      v2 = ( seed & 0x00007fffffff ) * scaleFactor - 1.0;
      r = v1 * v1 + v2 * v2;
    } while ( r > 1.0 );
    // radius < 1

    // remap v1 and v2 to two Gaussian numbers
    double noise = 1 / r; // approximation of sqrt(0.96) * sqrt(-log(r)/r);
    lms_input[1] = lms_sinus(1) + DATA_TYPE(noise * v2);
  }

  for ( k = 2 ; k < N ; k += 2 ) {
    double v1, v2, r;
    const double scaleFactor = 0.000000000931322574615478515625;
    do {
      // generate two random numbers between -1.0 and +1.0
      seed = seed * 1103515245 + 12345;
      v1 = ( seed & 0x00007fffffff ) * scaleFactor - 1.0;
      seed = seed * 1103515245 + 12345;
      v2 = ( seed & 0x00007fffffff ) * scaleFactor - 1.0;
      r = v1 * v1 + v2 * v2;
    } while ( r > 1.0 );
    // radius < 1

    // remap v1 and v2 to two Gaussian numbers
    double noise = 1 / r; // approximation of sqrt(0.96) * sqrt(-log(r)/r);
    lms_input[ k ] = lms_sinus(k) + DATA_TYPE(noise * v2);
    lms_input[ k + 1 ] = lms_sinus(k + 1) + DATA_TYPE(noise * v1);
  }

}


DATA_TYPE lms_calc( DATA_TYPE x,
                DATA_TYPE d,
                DATA_TYPE *b,
                DATA_TYPE l,
                DATA_TYPE mu,
                DATA_TYPE alpha,
                DATA_TYPE *history,
                DATA_TYPE *sigma )
{
  int i;

  // shift history

  for ( i = l ; i >= 1 ; i-- )
    history[ i ] = history[ i - 1 ];
  history[ 0 ] = x;

  // calculate filter
  DATA_TYPE y = 0.0;
  *sigma = alpha * x * x + ( 1 - alpha ) * ( *sigma );

  for ( i = 0 ; i <= l ; i++ )
    y += b[ i ] * history[ i ];

  // update coefficients
  DATA_TYPE e = mu * ( d - y ) / ( *sigma );

  for ( i = 0 ; i <= l ; i++ )
    b[ i ] += e * history[ i ];

  return y;
}


void lms(DATA_TYPE *b, DATA_TYPE *history, DATA_TYPE *lms_input,int L,DATA_TYPE *sigma,DATA_TYPE *lms_output){

    int i;
    const int N = args.N;

 for ( i = 0 ; i < N ; i++ ) {
    lms_output[i] = lms_calc( lms_input[ i ],
                              lms_input[ i + 1 ],
                              b, L, 0.02 / ( L + 1 ), 0.01,
                              history, sigma);
  }

}

void init_array_pathfinder(DATA_TYPE *data, DATA_TYPE **wall, DATA_TYPE *result){

    const int rows = args.N;
    const int cols = args.N;

    for(int n=0; n<rows; n++)
		wall[n]=data+cols*n;

    int seed = M_SEED;
	srand(seed);

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            wall[i][j] = DATA_TYPE(rand() % 10);
        }
    }

    for (int j = 0; j < cols; j++)
        result[j] = wall[0][j];
}

void init_array_bicg(DATA_TYPE *A, DATA_TYPE *p, DATA_TYPE *r)
{
	int i, j;

    const int NX = args.N;
    const int NY = args.N;

  	for (i = 0; i < NX; i++)
	{
    		r[i] = i * M_PI;

    		for (j = 0; j < NY; j++)
		{
      			A[i*NY + j] = ((DATA_TYPE) i*j) / NX;
		}
 	}
	
	for (i = 0; i < NY; i++)
	{
    		p[i] = i * M_PI;
	}
}


void init_array_mvt(DATA_TYPE* A, DATA_TYPE* x1, DATA_TYPE* x2, DATA_TYPE* y1, DATA_TYPE* y2)
{
	int i, j;
    const int N = args.N;

	for (i = 0; i < N; i++)
	{
		x1[i] = ((DATA_TYPE) i) / N;
		x2[i] = ((DATA_TYPE) i + 1) / N;
		y1[i] = ((DATA_TYPE) i + 3) / N;
		y2[i] = ((DATA_TYPE) i + 4) / N;
		for (j = 0; j < N; j++)
		{
			A[i*N + j] = ((DATA_TYPE) i*j) / N;
		}
	}
}


void init_array_gesummv(DATA_TYPE* A, DATA_TYPE* x)
{
  	int i, j;
    const int N = args.N;

 	for (i = 0; i < N; i++)
    {
    	x[i] = ((DATA_TYPE) i) / N;
      	
		for (j = 0; j < N; j++) 
		{
			A[i*N + j] = ((DATA_TYPE) i*j) / N;
		}
    }
}

void pathfinder(DATA_TYPE *data, DATA_TYPE **wall, DATA_TYPE *result, DATA_TYPE *src)
{
    DATA_TYPE *dst, *temp;
    DATA_TYPE min;

    const int cols = args.N;
    const int rows = args.N;

    dst = result;
    //src = new int[cols];

    for (int t = 0; t < rows-1; t++) {
        temp = src;
        src = dst;
        dst = temp;
 
        for(int n = 0; n < cols; n++){
          min = src[n];
          if (n > 0)
            min = MIN(min, src[n-1]);
          if (n < cols-1)
            min = MIN(min, src[n+1]);
          dst[n] = wall[t+1][n]+min;
        }
    }

}

void gesummv(DATA_TYPE *A, DATA_TYPE *B, DATA_TYPE *x, DATA_TYPE *y, DATA_TYPE *tmp)
{
	int i, j;
    const int N = args.N;

	for (i = 0; i < N; i++)
	{
		tmp[i] = 0;
		y[i] = 0;
		for (j = 0; j < N; j++)
		{
			tmp[i] = A[i*N + j] * x[j] + tmp[i];
			y[i] = B[i*N + j] * x[j] + y[i];
		}
		
		y[i] = ALPHA * tmp[i] + BETA * y[i];
	}
}


void bicg_cpu(DATA_TYPE* A, DATA_TYPE* r, DATA_TYPE* s, DATA_TYPE* p, DATA_TYPE* q)
{
	int i,j;
    const int NY = args.N;
    const int NX = args.N;

  	for (i = 0; i < NY; i++)
	{
		s[i] = 0.0;
	}

    for (i = 0; i < NX; i++)
    {
		q[i] = 0.0;
		for (j = 0; j < NY; j++)
	  	{
	    		s[j] = s[j] + r[i] * A[i*NY + j];
	    		q[i] = q[i] + A[i*NY + j] * p[j];
	  	}
	}
}


void runMvt(DATA_TYPE* a, DATA_TYPE* x1, DATA_TYPE* x2, DATA_TYPE* y1, DATA_TYPE* y2)
{
	int i, j;
    const int N = args.N;
	
	for (i=0; i<N; i++) 
	{
		for (j=0; j<N; j++) 
		{
       			x1[i] = x1[i] + a[i*N + j] * y1[j];
        	}
    	}
	
	for (i=0; i<N; i++) 
	{
		for (j=0; j<N; j++) 
		{
 		       	x2[i] = x2[i] + a[j*N + i] * y2[j];
      		}
    	}
}

void showHelp(){

    std::cout << "Usage: ./cpubench [-h or --help] [-v or --verbose] [-d or --datacount=<int>] [-i or --iterations=<int>] [-t or --test=<string>]" << std::endl;
    std::cout << "--help    Display help information" << std::endl;
    std::cout << "--verbose Activate verbosity. Default false " << std::endl;
    std::cout << "--datacount=<int>  Specify the problem size/data count for the test. Default 4096" << std::endl;
    std::cout << "--iterations=<int>  Specify the number of iterations to be performed. Default 1" << std::endl;
    std::cout << "--test=<string> Specify the benchmark. Possible values are MVT, BICG, GESUMMV and PATHFINDER. Default is MVT" << std::endl;

    exit(EXIT_SUCCESS);
}

inline bool parseArgs(ccpplexargs& args, int argc, char* argv[])
{
    while (1)
    {
        int arg;
        static struct option long_options[] = {{"help", no_argument, 0, 'h'},
            {"verbose", no_argument, 0, 'v'},
            {"datacount", required_argument, 0, 'd'},
            {"iterations", required_argument, 0, 'i'},
            {"test", required_argument, 0, 't'},
            {nullptr, 0, nullptr, 0}};
        int option_index = 0;
        arg = getopt_long(argc, argv, "hvd:i:t:", long_options, &option_index);
	if (arg == -1)
        {
            break;
        }

        switch (arg)
        {
        case 'h': args.help = true; 
        case 'v': args.verbosity = true; break;
        case 't':
            if (optarg)
            {
                if(strcmp(optarg,"MVT")==0)
                    args.test = MVT;
                else if (strcmp(optarg,"BICG")==0)
                    args.test = BICG;
                else if (strcmp(optarg,"GESUMMV")==0)
                    args.test = GESUMMV;
                else if (strcmp(optarg,"PATHFINDER")==0)
                    args.test = PATHFINDER;
                else if (strcmp(optarg,"LMS")==0)
                    args.test = LMS;
                else {
                    std::cout << "Invalid test chosen" << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            break;
        case 'd':
            if (optarg)
            {
                args.N = atoi(optarg);
            }
            break;
        case 'i':
            if (optarg)
            {
                args.iterations = atol(optarg);
            }
            break;
        default: return false;
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    double t_start, t_end;

    args.N = 4096;
    args.test = MVT;
    args.iterations = 1;
    args.verbosity = false;
    args.help = false;
    args.datasize = 0;

    parseArgs(args,argc,argv);

    if(args.help)
        showHelp();

    const int N = args.N;
    const int L = args.N;

    if(args.verbosity){

        std::string st = "MVT";
        args.datasize = (4*args.N+args.N*args.N)*sizeof(DATA_TYPE);

        if(args.test==BICG){
            st = "BICG";
            args.datasize = (4*args.N+args.N*args.N)*sizeof(DATA_TYPE);
        }
        else if(args.test==GESUMMV){
            st = "GESUMMV";
            args.datasize = (3*args.N+2*args.N*args.N)*sizeof(DATA_TYPE);
        }
        else if(args.test==PATHFINDER){
            st = "PATHFINDER";
            args.datasize = (2*args.N*args.N+2*args.N)*sizeof(DATA_TYPE);
        }
        else if(args.test==LMS){
            st = "LMS";
            args.datasize = (((L+1)*2) + ((N+1)*2))*sizeof(DATA_TYPE) + ( + (1+SAMPLING)*sizeof(double));
        }

	std::cout << "Will perform " << st << " test in " << args.iterations << " iteration(s)" << std::endl;
	std::cout << "Data is size for the problem is " << args.N << std::endl;
    std::cout << "Will touch slightly more than " << std::to_string(((float)args.datasize / (float)(1024*1024))) << " MiB of datasize" << std::endl;

    }

    DATA_TYPE sigma = DATA_TYPE(2.0);

        //MVT  BICG  GESUMMV 
	DATA_TYPE* a;   //a //a 
	DATA_TYPE* x1;  //r //b 
	DATA_TYPE* x2;  //s //x 
	DATA_TYPE* y_1; //p //y
	DATA_TYPE* y_2; //q //tmp

    //path finder exclusive variables
    DATA_TYPE* data;
    DATA_TYPE** wall;
    DATA_TYPE* result;
    DATA_TYPE* src;

    //LMS exclusive variables
    DATA_TYPE *b;
    DATA_TYPE *history;
    DATA_TYPE *lms_input;
    DATA_TYPE *lms_output;

    if(args.test==MVT){
        a = (DATA_TYPE*)malloc(N*N*sizeof(DATA_TYPE));
	    x1 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
	    x2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        y_1 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        y_2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
	    init_array_mvt(a, x1, x2, y_1, y_2);
    }
    else if(args.test==BICG){
        a = (DATA_TYPE*)malloc(N*N*sizeof(DATA_TYPE));
	    x1 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
	    x2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        y_1 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        y_2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        init_array_bicg(a,y_1,y_2);
    }
    else if(args.test==GESUMMV){
        a = (DATA_TYPE*)malloc(N*N*sizeof(DATA_TYPE));
        x1 = (DATA_TYPE*)malloc(N*N*sizeof(DATA_TYPE));
        x2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE)); 
        y_1 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        y_2 = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        init_array_gesummv(a,x2);
    }
    else if(args.test==PATHFINDER) {
        data = (DATA_TYPE*)malloc(N*N*sizeof(DATA_TYPE)); 
	    result = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE));
        src = (DATA_TYPE*)malloc(N*sizeof(DATA_TYPE)); 
	    wall = new DATA_TYPE*[N]; 
        init_array_pathfinder(data, wall, result);
    }
    else if(args.test==LMS){
        b = (DATA_TYPE*) malloc((L+1)*sizeof(DATA_TYPE));
        history = (DATA_TYPE*) malloc((L+1)*sizeof(DATA_TYPE));
        lms_input = (DATA_TYPE*) malloc((N+1)*sizeof(DATA_TYPE));
        lms_output = (DATA_TYPE*) malloc((N+1)*sizeof(DATA_TYPE));
        lms_init(lms_input,b,history);
    }
	
    if(args.verbosity)
        std::cout << "Start performing cpu compute tests..." << std::endl;

    for(size_t i = 0; i<args.iterations; i++){
        t_start = rtclock();

        if(args.test==MVT)
            runMvt(a, x1, x2, y_1, y_2);
        else if(args.test==BICG)
            bicg_cpu(a, x1, x2, y_1, y_2);
        else if(args.test==GESUMMV)
            gesummv(a,x1,x2,y_1,y_2);
        else if(args.test==PATHFINDER)
            pathfinder(data,wall,result,src);
        else if(args.test==LMS)
            lms(b,history,lms_input,L,&sigma,lms_output);

        t_end = rtclock();
        //fprintf(stdout, "CPU Runtime: %0.6lfs\n", t_end - t_start);
        PRINT_RESULT2(t_end - t_start);
    }

    if(args.verbosity)
        std::cout << "Finishing performing cpu compute tests" << std::endl;
    
    if(args.test!=PATHFINDER && args.test!=LMS){
        free(a);
        free(x1);
        free(x2);
        free(y_1);
        free(y_2);
    }else if(args.test==PATHFINDER){
        free(src);
        free(result);
        delete [] wall;
        free(data);
    }else if (args.test==LMS){
        free(lms_input);
        free(lms_output);
        free(b);
        free(history);
    }

  	return EXIT_SUCCESS;
}

