//
//  main.c
//  MandelbrotSet
//
//  Created by Elgiz Bağcılar on 02.07.20.
//  Copyright © 2020 Elgiz Bağcılar. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <OpenCL/opencl.h>
#include "mandelbrot_kernel.cl.h"
#include "lodepng.h"

#define GRID_X 2048
#define GRID_Y 2048


int main(int argc, const char * argv[]) {
    
    char name[128];
        
    dispatch_queue_t queue =
              gcl_create_dispatch_queue(CL_DEVICE_TYPE_GPU, NULL);


    if (queue == NULL) {
       queue = gcl_create_dispatch_queue(CL_DEVICE_TYPE_CPU, NULL);
    }
    
    cl_device_id gpu = gcl_get_device_id_with_dispatch_queue(queue);
    clGetDeviceInfo(gpu, CL_DEVICE_NAME, 128, name, NULL);
    
    float* rowValues_host = (float*)malloc(sizeof(cl_float) * GRID_X * GRID_Y);
    float* colValues_host = (float*)malloc(sizeof(cl_float) * GRID_X * GRID_Y);
    
    int row,col;
    
    //Map the pixels in between -2 and 2
    for (row = 0; row < GRID_X; row++) {
        for (col = 0; col < GRID_Y; col++) {
            rowValues_host[row * GRID_X + col] = (col * 4.0)/GRID_X - 2.0;
            colValues_host[row * GRID_X + col] = (row * 4.0)/GRID_Y - 2.0;
        }
    }
    
    unsigned char* pixVec_host = (unsigned char*)malloc(sizeof(cl_uchar) * GRID_X * GRID_Y * 3);
    
    
    void* mem_in_x = gcl_malloc(sizeof(cl_float) * GRID_X * GRID_Y, rowValues_host,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
    void* mem_in_y = gcl_malloc(sizeof(cl_float) * GRID_X * GRID_Y, colValues_host,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR);
    void* mem_out = gcl_malloc(sizeof(cl_uchar) * GRID_X * GRID_Y * 3, NULL, CL_MEM_WRITE_ONLY);

    // Comment out for giving different coefficients
    //float const_x = atof(argv[1]);
    //float const_y = atof(argv[2]);
    
    float const_x = 0.0;
    float const_y = 0.0;

    dispatch_sync(queue, ^{
       size_t wgs;
       gcl_get_kernel_block_workgroup_info(computeFractal_kernel,
                                           CL_KERNEL_WORK_GROUP_SIZE,
                                           sizeof(wgs), &wgs, NULL);
       
       cl_ndrange range = {1, {0, 0, 0}, {GRID_X * GRID_Y, 0, 0}, {wgs, 0, 0}};
       
       computeFractal_kernel(&range,(cl_float*)mem_in_x,(cl_float*)mem_in_y, (cl_uchar*)mem_out, (cl_float)const_x, (cl_float)const_y);
    

       gcl_memcpy(pixVec_host, mem_out, sizeof(cl_uchar) * GRID_X * GRID_Y * 3);

    });

    unsigned error = lodepng::encode("juliaset_opencl.png", pixVec_host, 2048, 2048, LCT_RGB, 8);
    if(error) printf("error %u: %s\n", error, lodepng_error_text(error));

    gcl_free(mem_in_x);
    gcl_free(mem_in_y);
    gcl_free(mem_out);

    free(rowValues_host);
    free(colValues_host);

    dispatch_release(queue);
}
