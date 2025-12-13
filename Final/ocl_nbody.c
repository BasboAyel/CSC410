// OpenCL for N-body simulation

#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_BODIES 1000
#define DT (60 * 60 * 24)

typedef struct {
    float x, y;
    float vx, vy;
    float mass;
} Body;

// -------------------- KERNELS --------------------
const char *kernel_source =
"typedef struct {                       \n"
"    float x, y;                        \n"
"    float vx, vy;                      \n"
"    float mass;                        \n"
"} Body;                                \n"
"__kernel void compute_forces(          \n"
"   __global const Body *bodies,        \n"
"   __global float *fx,                 \n"
"   __global float *fy,                 \n"
"   const int num_bodies) {             \n"
"    int i = get_global_id(0);           \n"
"    if (i >= num_bodies) return;       \n"
"    float fx_i = 0.0f;                 \n"
"    float fy_i = 0.0f;                 \n"
"    for (int j = 0; j < num_bodies; j++) { \n"
"        if (i != j) {                  \n"
"            float dx = bodies[j].x - bodies[i].x; \n"
"            float dy = bodies[j].y - bodies[i].y; \n"
"            float dist2 = dx*dx + dy*dy + 1e6f;    \n"
"            float dist = sqrt(dist2);              \n"
"            float f = 6.67430e-11f * bodies[i].mass * bodies[j].mass / dist2; \n"
"            fx_i += f * dx / dist; \n"
"            fy_i += f * dy / dist; \n"
"        }                              \n"
"    }                                  \n"
"    fx[i] = fx_i;                      \n"
"    fy[i] = fy_i;                      \n"
"}                                      \n";

const char *update_kernel_source =
"typedef struct {                       \n"
"    float x, y;                        \n"
"    float vx, vy;                      \n"
"    float mass;                        \n"
"} Body;                                \n"
"__kernel void update_bodies(           \n"
"   __global Body *bodies,              \n"
"   __global const float *fx,           \n"
"   __global const float *fy,           \n"
"   const float dt,                     \n"
"   const int num_bodies) {             \n"
"    int i = get_global_id(0);           \n"
"    if (i >= num_bodies) return;       \n"
"    bodies[i].vx += fx[i] / bodies[i].mass * dt; \n"
"    bodies[i].vy += fy[i] / bodies[i].mass * dt; \n"
"    bodies[i].x  += bodies[i].vx * dt; \n"
"    bodies[i].y  += bodies[i].vy * dt; \n"
"}                                      \n";

int main() {

    Body bodies[NUM_BODIES];

    for (int i = 0; i < NUM_BODIES; i++) {
        bodies[i].x = rand() % 1000000000;
        bodies[i].y = rand() % 1000000000;
        bodies[i].vx = 0.0f;
        bodies[i].vy = 0.0f;
        bodies[i].mass = (rand() % 100 + 1) * 1e24f;
    }

    int num_bodies = NUM_BODIES;
    float dt = DT;

    cl_int err;
    cl_platform_id platform;
    cl_device_id device;

    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);

    cl_mem buffer_bodies = clCreateBuffer(
        context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(Body) * NUM_BODIES, bodies, &err);

    cl_mem buffer_fx = clCreateBuffer(
        context, CL_MEM_READ_WRITE,
        sizeof(float) * NUM_BODIES, NULL, &err);

    cl_mem buffer_fy = clCreateBuffer(
        context, CL_MEM_READ_WRITE,
        sizeof(float) * NUM_BODIES, NULL, &err);

    cl_program program = clCreateProgramWithSource(
        context, 1, &kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "compute_forces", &err);

    cl_program update_program = clCreateProgramWithSource(
        context, 1, &update_kernel_source, NULL, &err);
    clBuildProgram(update_program, 1, &device, NULL, NULL, NULL);
    cl_kernel update_kernel = clCreateKernel(update_program, "update_bodies", &err);

    size_t global_size = NUM_BODIES;

    for (int step = 0; step < 1000; step++) {

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_bodies);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_fx);
        clSetKernelArg(kernel, 2, sizeof(cl_mem), &buffer_fy);
        clSetKernelArg(kernel, 3, sizeof(int), &num_bodies);

        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);

        clSetKernelArg(update_kernel, 0, sizeof(cl_mem), &buffer_bodies);
        clSetKernelArg(update_kernel, 1, sizeof(cl_mem), &buffer_fx);
        clSetKernelArg(update_kernel, 2, sizeof(cl_mem), &buffer_fy);
        clSetKernelArg(update_kernel, 3, sizeof(float), &dt);
        clSetKernelArg(update_kernel, 4, sizeof(int), &num_bodies);

        clEnqueueNDRangeKernel(queue, update_kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);

        clFinish(queue);
    }

    clEnqueueReadBuffer(queue, buffer_bodies, CL_TRUE, 0,
                        sizeof(Body) * NUM_BODIES, bodies, 0, NULL, NULL);

    for (int i = 0; i < 5; i++) {
        printf("Body %d: (%.2f, %.2f)\n", i, bodies[i].x, bodies[i].y);
    }

    clReleaseMemObject(buffer_bodies);
    clReleaseMemObject(buffer_fx);
    clReleaseMemObject(buffer_fy);
    clReleaseKernel(kernel);
    clReleaseKernel(update_kernel);
    clReleaseProgram(program);
    clReleaseProgram(update_program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}
