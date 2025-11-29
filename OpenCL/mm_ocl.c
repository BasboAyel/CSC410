// mm_ocl.c - Matrix Multiplication using OpenCL
#define CL_TARGET_OPENCL_VERSION 120  
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>
#include <time.h>

#define MATRIX_SIZE 512

// Kernel source for matrix multiplication
const char* kernelSource = 
"__kernel void matrix_mult(__global float* A, __global float* B, __global float* C, int size) {\n"
"    int row = get_global_id(0);\n"
"    int col = get_global_id(1);\n"
"    \n"
"    if (row < size && col < size) {\n"
"        float sum = 0.0f;\n"
"        for (int k = 0; k < size; k++) {\n"
"            sum += A[row * size + k] * B[k * size + col];\n"
"        }\n"
"        C[row * size + col] = sum;\n"
"    }\n"
"}\n";

// Sequential matrix multiplication for comparison
void matrix_mult_sequential(float* A, float* B, float* C, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float sum = 0.0f;
            for (int k = 0; k < size; k++) {
                sum += A[i * size + k] * B[k * size + j];
            }
            C[i * size + j] = sum;
        }
    }
}

int main() {
    printf("Starting Matrix Multiplication Benchmark...\n");
    
    // Initialize matrices
    int total_elements = MATRIX_SIZE * MATRIX_SIZE;
    float* A = (float*)malloc(total_elements * sizeof(float));
    //float* A = (float*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(float));
    float* B = (float*)malloc(total_elements * sizeof(float));
    float* C_seq = (float*)malloc(total_elements * sizeof(float));
    float* C_ocl = (float*)malloc(total_elements * sizeof(float));
    
    if (!A || !B || !C_seq || !C_ocl) {
        printf("Memory allocation failed!\n");
        return -1;
    }
    
    // Initialize with simple values
    printf("Initializing matrices...\n");
    for (int i = 0; i < total_elements; i++) {
        A[i] = (float)(i % MATRIX_SIZE) + 1.0f;  // 1, 2, 3, ...
        B[i] = (float)(i / MATRIX_SIZE) + 1.0f;  // 1, 1, 1, ..., 2, 2, 2, ...
    }
    
    // Time sequential version
    printf("Running sequential version...\n");
    clock_t start_seq = clock();
    matrix_mult_sequential(A, B, C_seq, MATRIX_SIZE);
    clock_t end_seq = clock();
    double time_seq = ((double)(end_seq - start_seq)) / CLOCKS_PER_SEC * 1000.0;
    
    // OpenCL setup
    cl_platform_id platform_id;
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem bufferA, bufferB, bufferC;
    cl_int ret;
    
    printf("Initializing OpenCL...\n");
    
    // Get platform and device
    ret = clGetPlatformIDs(1, &platform_id, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to get platform ID: %d\n", ret);
        return -1;
    }
    
    ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to get GPU device, trying CPU...\n");
        ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
        if (ret != CL_SUCCESS) {
            printf("Failed to get any OpenCL device: %d\n", ret);
            return -1;
        }
    }
    
    // Create context
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create context: %d\n", ret);
        return -1;
    }
    
    // Create command queue with profiling
    command_queue = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create command queue: %d\n", ret);
        clReleaseContext(context);
        return -1;
    }
    
    printf("OpenCL initialized successfully\n");
    
    // Create buffers
    bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY, total_elements * sizeof(float), NULL, &ret);
    bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY, total_elements * sizeof(float), NULL, &ret);
    bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, total_elements * sizeof(float), NULL, &ret);
    
    if (ret != CL_SUCCESS) {
        printf("Failed to create buffers: %d\n", ret);
        clReleaseCommandQueue(command_queue);
        clReleaseContext(context);
        return -1;
    }
    
    // Copy data to device
    printf("Copying data to device...\n");
    ret = clEnqueueWriteBuffer(command_queue, bufferA, CL_TRUE, 0, total_elements * sizeof(float), A, 0, NULL, NULL);
    ret |= clEnqueueWriteBuffer(command_queue, bufferB, CL_TRUE, 0, total_elements * sizeof(float), B, 0, NULL, NULL);
    
    if (ret != CL_SUCCESS) {
        printf("Failed to write buffers: %d\n", ret);
        goto cleanup;
    }
    
    // Create and build program
    printf("Building kernel...\n");
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create program: %d\n", ret);
        goto cleanup;
    }
    
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to build program: %d\n", ret);
        
        // Get build log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build log:\n%s\n", log);
        free(log);
        
        goto cleanup;
    }
    
    // Create kernel
    kernel = clCreateKernel(program, "matrix_mult", &ret);
    if (ret != CL_SUCCESS) {
        printf("Failed to create kernel: %d\n", ret);
        goto cleanup;
    }
    
    // Set kernel arguments - THIS IS THE PART YOU ASKED ABOUT
    printf("Setting kernel arguments...\n");
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    ret |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    ret |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    int matrix_size = MATRIX_SIZE;
    ret |= clSetKernelArg(kernel, 3, sizeof(int), &matrix_size);
    
    if (ret != CL_SUCCESS) {
        printf("Failed to set kernel arguments: %d\n", ret);
        goto cleanup;
    }
    
    // Set work sizes for 2D kernel execution
    size_t global_work_size[2] = {MATRIX_SIZE, MATRIX_SIZE};
    
    printf("Executing kernel...\n");
    
    // Execute kernel with timing
    cl_event kernel_event;
    ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, NULL, 0, NULL, &kernel_event);
    if (ret != CL_SUCCESS) {
        printf("Failed to execute kernel: %d\n", ret);
        goto cleanup;
    }
    
    clWaitForEvents(1, &kernel_event);
    
    // Get kernel execution time
    cl_ulong start_time, end_time;
    ret = clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_time, NULL);
    ret |= clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_time, NULL);
    
    double time_ocl = 0.0;
    if (ret == CL_SUCCESS) {
        time_ocl = (end_time - start_time) * 1e-6; // Convert to milliseconds
    } else {
        printf("Profiling not available, using fallback timing\n");
        clock_t start_ocl = clock();
        clWaitForEvents(1, &kernel_event);
        clock_t end_ocl = clock();
        time_ocl = ((double)(end_ocl - start_ocl)) / CLOCKS_PER_SEC * 1000.0;
    }
    
    // Read results back
    printf("Reading results from device...\n");
    ret = clEnqueueReadBuffer(command_queue, bufferC, CL_TRUE, 0, total_elements * sizeof(float), C_ocl, 0, NULL, NULL);
    if (ret != CL_SUCCESS) {
        printf("Failed to read results: %d\n", ret);
        goto cleanup;
    }
    
    // Verify results
    printf("Verifying results...\n");
    int errors = 0;
    for (int i = 0; i < 100; i++) { // Check first 100 elements only
        if (fabs(C_seq[i] - C_ocl[i]) > 0.001f) {
            errors++;
            if (errors <= 5) {
                printf("Mismatch at index %d: Sequential=%f, OpenCL=%f\n", i, C_seq[i], C_ocl[i]);
            }
        }
    }
    
    // Print results
    printf("\n=== MATRIX MULTIPLICATION RESULTS ===\n");
    printf("Matrix Size: %d x %d\n", MATRIX_SIZE, MATRIX_SIZE);
    printf("Sequential Time: %.2f ms\n", time_seq);
    printf("OpenCL Time: %.2f ms\n", time_ocl);
    
    if (time_ocl > 0) {
        printf("Speedup: %.2fx\n", time_seq / time_ocl);
    }
    
    printf("Verification Errors: %d\n", errors);
    
    // Print timing table for assignment
    printf("\n=== TIMING COMPARISON ===\n");
    printf("| Time (ms)    | Sequential | OpenCL    |\n");
    printf("|--------------|------------|-----------|\n");
    printf("| Execution    | %10.2f | %9.2f |\n", time_seq, time_ocl);
    
    if (errors == 0) {
        printf("\nResults verified successfully!\n");
    } else {
        printf("\nFound %d verification errors\n", errors);
    }

cleanup:
    // Cleanup
    if (kernel_event) clReleaseEvent(kernel_event);
    if (bufferA) clReleaseMemObject(bufferA);
    if (bufferB) clReleaseMemObject(bufferB);
    if (bufferC) clReleaseMemObject(bufferC);
    if (kernel) clReleaseKernel(kernel);
    if (program) clReleaseProgram(program);
    if (command_queue) clReleaseCommandQueue(command_queue);
    if (context) clReleaseContext(context);
    
    free(A);
    free(B);
    free(C_seq);
    free(C_ocl);
    
    return 0;
}