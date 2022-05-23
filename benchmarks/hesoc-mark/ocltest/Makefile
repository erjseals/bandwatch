EXECUTABLE := oclinterf
CFILES := oclinterf.cpp

#IMPORTANT:
#include and library location can change according to your system:
#be sure to fix them!

OpenCL_SDK=/usr/local/cuda-10.0/targets/x86_64-linux

INCLUDE=-I"${OpenCL_SDK}/include" -I"${OpenCL_SDK}/include/CL"
LIBPATH=-L"/usr/local/cuda/targets/x86_64-linux/lib/"
LIB=-lOpenCL -lm

all: oclinterf oclmeasure

oclinterf:
	g++ --std=gnu++11 -O3 ${INCLUDE} ${LIBPATH} ${CFILES} -o ${EXECUTABLE} ${LIB}

oclmeasure:
	g++ --std=gnu++11 -O3 ${INCLUDE} ${LIBPATH} oclmeasure.cpp -o oclmeasure ${LIB}

clean:
	rm -f *~
	rm -f oclinterf
	rm -f oclmeasure

