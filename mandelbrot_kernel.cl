#pragma OPENCL EXTENSION cl_khr_fp64 : enable

kernel void computeFractal(global float* row, global float* col, global uchar* pix_vec, private float c_x, private float c_y)
{
    int i = get_global_id(0);

    if(i < 2048*2048){
        int iteration=0;
        int max_ite=80;
        
        //for mandelbrot set
        float t_x = row[i];
        float t_y = col[i];
        
        //for zoomed parts of the mandelbrot set
        //float t_x = c_x;
        //float t_y = c_y;
        
        while((row[i]*row[i] + col[i]*col[i]) < 100 && iteration < max_ite){
            float temp_row = (row[i] * row[i]) - (col[i] * col[i]);
            col[i] = (2 * row[i] * col[i]) + t_y;
            row[i] = temp_row + t_x;
            ++iteration;
        }
        if (iteration < max_ite) {
            pix_vec[i*3]=(unsigned char)(iteration*255/(max_ite));
            pix_vec[i*3+1]=0;
            pix_vec[i*3+2]=0;
        }
        else {
            pix_vec[i*3]= (unsigned char)0;
            pix_vec[i*3+1]=(unsigned char)0;
            pix_vec[i*3+2]=(unsigned char)0;
        }

    }
}
