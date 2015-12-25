//
// oclprogs.h --- signal processing algorithms in OpenCL
//
// Copyright (c) 2015 Juergen Kahrs
//
// Permission is hereby granted, free of charge, to any person 
// obtaining a copy of this software and associated documentation 
// files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, 
// merge, publish, distribute, sublicense, and/or sell copies of 
// the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be 
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE 
// OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef _OCLPROGS_H
#define _OCLPROGS_H 1

// This file specifies the interface to the signal processing
// algorithms that run on a graphics card (GPU). The interface is
// implemented as a library that is built upon the clFFT and the
// OpenCL libraries. This specification and its library try to
// shield the user of the library from all GPU terminology by
// encapsulating the GPU-internals inside the library.

// You must invoke start_ocl before using the library.
void start_ocl(void);
// After invoking stop_ocl the library cannot be used any more.
void stop_ocl(void);
// info_ocl runs several tests and prints the results to stdout.
void info_ocl(void);

// Define an opaque data type that hides clFFT plans behind syntactic sugar. 
typedef void * oclHandleType;
 
// You must invoke create_clFFT_plan before doing signal processing.
// The following limitations are preliminary and some will be resolved later.
// All plans use dimension 1.
// All numbers are floats.
// All data arrays are complex interleaved.
oclHandleType create_clFFT_plan(
  size_t array_length,    // number of complex samples, not number of bytes
  size_t batch_size,      // number of arrays to process simultaneously
  float scale,            // factor to multiply on each resulting value
  unsigned short platform,// 0, enumerates OpenCL drivers by manufacturer
  unsigned short device   // 0, enumerates cards of the same manufacturer
);

// Perform FFT on input data. The result is returned in output array.
// Pass identical input and output pointers if you want results in-place.
// Pass unequal input and output pointers if you want results out-of-place.
int execute_clFFT_plan(
  oclHandleType handle,
  int direction,     // -1 = forward, +1 = backward
  void * input,
  void * output
);

int destroy_clFFT_plan(oclHandleType * handle_p);

#endif /* _OCLPROGS_H */

