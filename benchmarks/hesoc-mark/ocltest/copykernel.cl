
__kernel void copykernel(__global float *A, __global float *B) 
{
	int i = get_global_id(0);
	A[i] = B[i];
}

