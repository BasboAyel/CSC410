// Intro to OpenCL - Setting up
    // Query platforms and devices available
    // Create contex and command queue

#define CL_TARGET_OPENCL_VERSION 120  
#include <stdio.h>
#include <CL/cl.h>

int main() {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue command_queue;
    cl_int ret;

    // Get the first available platform
    ret = clGetPlatformIDs(1, &platform, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to find OpenCL platform\n");
        return -1;
    }

    // Get the first available device on the platform
    ret = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to find OpenCL device\n");
        return -1;
    }

    // Create context for the device
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create context\n");
        return -1;
    }

    // Create command queue
    command_queue = clCreateCommandQueue(context, device, 0, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create command queue\n");
        clReleaseContext(context);
        return -1;
    }

    printf("OpenCL Platform and Device initialized successfully\n");

    clReleaseCommandQueue(command_queue);
    clReleaseContext(context);

    return 0;
}
