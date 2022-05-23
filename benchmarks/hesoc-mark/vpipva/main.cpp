/*
* Copyright (c) 2019, NVIDIA CORPORATION. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*  * Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*  * Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*  * Neither the name of NVIDIA CORPORATION nor the names of its
*    contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vpi/Event.h>
#include <vpi/Image.h>
#include <vpi/Pyramid.h>
#include <vpi/Stream.h>
#include <vpi/algo/GaussianImageFilter.h>
#include <vpi/algo/GaussianPyramidGenerator.h>
#include <vpi/algo/StereoDisparityEstimator.h>
#include <vpi/algo/ImageConvolver.h>
#include <vpi/algo/SeparableImageConvolver.h>
#include <vpi/algo/BoxImageFilter.h>
#include <vpi/algo/KLTBoundingBoxTracker.h>
#include <iostream>
#include <opencv2/imgcodecs.hpp>
#include <stdio.h>
#include <time.h>
#include <unistd.h> /* for sleep() */
#include <bits/stdc++.h> 
#include <sys/time.h> 
using namespace std; 

#define MAX_D 3200
#define MIN_D 2400

#define VERBOSE false

#define CHECK_STATUS(STMT)                                      \
    do                                                          \
    {                                                           \
        VPIStatus status = (STMT);                              \
        if (status != VPI_SUCCESS)                              \
        {                                                       \
            throw std::runtime_error(vpiStatusGetName(status)); \
        }                                                       \
    } while (0);




int main(int argc, char *argv[])
{
	int iterations=atoi(argv[1]);
	std::string strDevType = argv[2];
	int nStreams=atoi(argv[3]);
    std::string strAlgType[nStreams];
	VPIStream streams[nStreams];
    int retval = 0;
    int opt[nStreams];

    struct timespec start, end;
    VPIEvent evStart[nStreams];
    VPIEvent evEnd[nStreams];
    VPIPyramid outputPyramid[nStreams];
    VPIImage left[nStreams];
    VPIImage right[nStreams];
    VPIImage disparity[nStreams];
    VPIPayload stereo[nStreams];
    VPIImage image[nStreams];
    VPIImage outputImage[nStreams];
	VPIImage imageSeparable[nStreams];
	VPIImage outputSeparable[nStreams];
    try
    {
		int width = MAX_D;
		int height = MIN_D;
 
        // Process the device type
        VPIDeviceType devType;

        if (strDevType == "cpu")
        {
            devType = VPI_DEVICE_TYPE_CPU;
        }
        else if (strDevType == "cuda")
        {
            devType = VPI_DEVICE_TYPE_CUDA;
        }
        else if (strDevType == "pva")
        {
            devType = VPI_DEVICE_TYPE_PVA;
        }
        else
        {
            throw std::runtime_error("Backend '" + strDevType +
                                     "' not recognized, it must be either cpu, cuda or pva.");
        }

        // Create the stream for the given backend.
		for(int i=0;i<nStreams;i++){
        	CHECK_STATUS(vpiStreamCreate(devType, &streams[i]));
			opt[i] = atoi(argv[4+i]);
		    CHECK_STATUS(vpiEventCreate(0, &evStart[i]));
		    CHECK_STATUS(vpiEventCreate(0, &evEnd[i]));
		}
        VPIImageType imgType = VPI_IMAGE_TYPE_Y16;

		float kernel[3 * 3] = {1,0,-1,
							   0,0,0,
							  -1,0,1};

		for (int i = 0; i < 9; ++i)
		{
			kernel[i] /= 1+FLT_EPSILON;
		}
		float sobel_row[7] = {1/64.f, 6/64.f, 15/64.f, 20/64.f, 15/64.f, 6/64.f, 1/64.f};
		float sobel_col[7] = {1/64.f, 6/64.f, 15/64.f, 20/64.f, 15/64.f, 6/64.f, 1/64.f};
    	VPIStereoDisparityEstimatorParams params;
		params.windowSize   = 5;
		params.maxDisparity = 64;

		//init
		for(int i=0;i<nStreams;i++){
			CHECK_STATUS(vpiImageCreate(480, 270, VPI_IMAGE_TYPE_Y16, 0, &right[i]));
			CHECK_STATUS(vpiImageCreate(480, 270, VPI_IMAGE_TYPE_Y16, 0, &left[i]));
	   		CHECK_STATUS(vpiImageCreate(480, 270, VPI_IMAGE_TYPE_Y16, 0, &disparity[i]));
			CHECK_STATUS(vpiPyramidCreate(width, height, VPI_IMAGE_TYPE_Y16, 3, 0.5, 0, &outputPyramid[i]));
    		CHECK_STATUS(vpiImageCreate(width, height, VPI_IMAGE_TYPE_Y16I, 0, &imageSeparable[i]));
    		CHECK_STATUS(vpiImageCreate(width, height, VPI_IMAGE_TYPE_Y16I, 0, &outputSeparable[i]));
    		CHECK_STATUS(vpiImageCreate(width, height, VPI_IMAGE_TYPE_Y16, 0, &image[i]));
	    	CHECK_STATUS(vpiImageCreate(width, height, VPI_IMAGE_TYPE_Y16, 0, &outputImage[i]));
			CHECK_STATUS(vpiCreateStereoDisparityEstimator(streams[i], 480, 270, VPI_IMAGE_TYPE_Y16, params.maxDisparity, &stereo[i])); 
		}
		
		std::cout << "Start of vpi tests " << std::endl;
		for(int z=0;z<iterations;z++){

			clock_gettime(CLOCK_MONOTONIC, &start);
			for(int i=0;i<nStreams;i++){

				if (opt[i] == 0) //Stereo Disparity Estimator - OK
				{	
					strAlgType[i]="Stereo Disparity Estimator";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
					CHECK_STATUS(vpiSubmitStereoDisparityEstimator(stereo[i], left[i], right[i], disparity[i], &params));
				}else if (opt[i] == 1) //KLT Bounding Box Tracker	- NOT OK	
				{
					strAlgType[i]="KLT Bounding Box Tracker";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));

				}else if (opt[i] == 2) //Gaussian Pyramid Generator - OK	
				{
					strAlgType[i]="Gaussian Pyramid Generator";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
					CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(streams[i], image[i], outputPyramid[i]));

				}else if (opt[i] == 3) //Image Convolver - OK
				{
					strAlgType[i]="Image Convolver";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
					CHECK_STATUS(vpiSubmitImageConvolver(streams[i], image[i], outputImage[i], kernel, 3, 3, VPI_BOUNDARY_COND_ZERO));

				}else if (opt[i] == 4) //Separable Image Convolver	-OK
				{
					strAlgType[i]="Separable Image Convolver";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
					CHECK_STATUS(vpiSubmitSeparableImageConvolver(streams[i], imageSeparable[i], outputSeparable[i], sobel_row, 7, sobel_col, 7, VPI_BOUNDARY_COND_ZERO));

				}else if (opt[i] == 5) //Box Image Filter	- OK
				{
					strAlgType[i]="Box Image Filter";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
		   			CHECK_STATUS(vpiSubmitBoxImageFilter(streams[i], imageSeparable[i], outputSeparable[i], 7, 7, VPI_BOUNDARY_COND_ZERO));

				}else if (opt[i] == 6) //Gaussian Image Filter	- OK
				{
					strAlgType[i]="Gaussian Image Filter";
					CHECK_STATUS(vpiEventRecord(evStart[i], streams[i]));
					CHECK_STATUS(vpiSubmitGaussianImageFilter(streams[i], image[i], outputImage[i], 5, 5, 1, 1, VPI_BOUNDARY_COND_ZERO));
				}
				CHECK_STATUS(vpiEventRecord(evEnd[i], streams[i]));
			}

			for(int i=0;i<nStreams;i++)
				vpiEventSync(evEnd[i]);

			clock_gettime(CLOCK_MONOTONIC, &end);
			for(int i=0;i<nStreams;i++){
				float elapsedTotalMS;
				CHECK_STATUS(vpiEventElapsedTime(evStart[i], evEnd[i], &elapsedTotalMS));
				if(VERBOSE)
					std::cout << "Dev type: "<< strDevType <<" | Alg: " << strAlgType[i] << " ("<<width<<"x"<<height<<")"<<" | elapsed time " << elapsedTotalMS << "ms \n";
			}
			// Calculating total time taken by the program. 
			double time_taken; 
			time_taken = (end.tv_sec - start.tv_sec) * 1e9; 
			time_taken = (time_taken + (end.tv_nsec - start.tv_nsec)) * 1e-9; 
		  
			if(VERBOSE){
			cout << "Time taken by program is : " << fixed 
				 << time_taken*1000 << setprecision(6); 
			cout << " msec" << " | iteration number " << z << endl; 
			}

		}
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retval = 1;
    }

	std::cout << "End of vpi tests " << std::endl;

    // 4. Clean up -----------------------------------

	for(int i=0;i<nStreams;i++){
    	vpiStreamDestroy(streams[i]);
		vpiEventDestroy(evStart[i]);
		vpiEventDestroy(evEnd[i]);
    	vpiPyramidDestroy(outputPyramid[i]);
		vpiImageDestroy(left[i]);
		vpiImageDestroy(right[i]);
    	vpiImageDestroy(image[i]);
    	vpiImageDestroy(outputImage[i]);
    	vpiImageDestroy(imageSeparable[i]);
    	vpiImageDestroy(outputSeparable[i]);
		vpiImageDestroy(disparity[i]);
    	//vpiPayloadDestroy(stereo[i]); //SF
	}
    return retval;
}
