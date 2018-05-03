/*
 * targetDP.h: definitions, macros and declarations for targetDP.
 * Alan Gray
 * 
 * Copyright 2015 The University of Edinburgh
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TDP_INCLUDED_
#define TDP_INCLUDED_

/* KS. Additions */

#include "cuda_stub_api.h" /* To be merged with target_api.h */
#include "target_api.h"

/* KS. End additions */


/* Main settings */

#ifndef VVL

#define VVL_CUDA 1 /* virtual vector length for TARGETDP CUDA (usually 1) */
#define VVL_C 1 /* virtual vector length for TARGETDP C (usually 1 for AoS */
              /*  or a small multiple of the hardware vector length for SoA)*/

#else /* allow this to be overwritten with compilation option -DVVL=N */

#define VVL_CUDA VVL
#define VVL_C VVL

#endif

/* End main settings */


#ifdef __NVCC__ /* CUDA */


/*
 * CUDA Settings 
 */

#define DEFAULT_TPB 128 /* default threads per block */

/* Instruction-level-parallelism vector length  - to be tuned to hardware*/
#ifndef VVL
#define VVL VVL_CUDA
#endif

/*
 * Language Extensions 
 */

/* The __targetLaunch__ syntax is used to launch a function across 
 * a data parallel target architecture. */
#define __targetLaunch__(extent) \
  <<<((extent/VVL)+DEFAULT_TPB-1)/DEFAULT_TPB,DEFAULT_TPB>>>

/* as above but with stride of 1 */
#define __targetLaunchNoStride__(extent) \
  <<<((extent)+DEFAULT_TPB-1)/DEFAULT_TPB,DEFAULT_TPB>>>
  

/* Thread-level-parallelism execution macro */

/* The __targetTLP__ syntax is used, within a __targetEntry__ function, to
 * specify that the proceeding block of code should be executed in parallel and
 * mapped to thread level parallelism (TLP). Note that he behaviour of this op-
 * eration depends on the defined virtual vector length (VVL), which controls the
 * lower-level Instruction Level Parallelism (ILP)  */
#define __targetTLP__(simtIndex,extent) \
  simtIndex = VVL*(blockIdx.x*blockDim.x+threadIdx.x);	\
  if (simtIndex < extent)

/* as above but with stride of 1 */
#define __targetTLPNoStride__(simtIndex,extent) \
  simtIndex = (blockIdx.x*blockDim.x+threadIdx.x);	\
  if (simtIndex < extent)


/* Instruction-level-parallelism execution macro */
/* The __targetILP__ syntax is used, within a __targetTLP__ region, to specify
 * that the proceeding block of code should be executed in parallel and mapped to
 * instruction level parallelism (ILP), where the extent of the ILP is defined by the
 * virtual vector length (VVL) in the targetDP implementation. */
#if VVL == 1
#define __targetILP__(vecIndex)  vecIndex = 0;
#else
#define __targetILP__(vecIndex)  for (vecIndex = 0; vecIndex < VVL; vecIndex++) 
#endif


/* Functions */

/* The targetConstAddress function provides the target address for a constant
 *  object. */
#define targetConstAddress(addr_of_ptr,const_object) \
  cudaGetSymbolAddress(addr_of_ptr, const_object); \
  checkTargetError("__getTargetConstantAddress__"); 

/* The copyConstToTarget function copies data from the host to the target, 
 * where the data will remain constant (read-only) during the execution of 
 * functions on the target. */
#define copyConstToTarget(data_d, data, size) \
  cudaMemcpyToSymbol(*data_d, (const void*) data, size, 0,cudaMemcpyHostToDevice); \
   checkTargetError("copyConstToTarget"); 

/* The copyConstFromTarget function copies data from a constant data location
 *  on the target to the host. */
#define copyConstFromTarget(data, data_d, size) \
  cudaMemcpyFromSymbol((void*) data, *data_d, size, 0,cudaMemcpyDeviceToHost); \
   checkTargetError("__copyConstantFromTarget__"); 





#else /* C versions of the above*/

/* SEE ABOVE FOR DOCUMENTATION */

/* Settings */

/* Instruction-level-parallelism vector length  - to be tuned to hardware*/

#ifndef VVL
#define VVL VVL_C
#endif

/* Language Extensions */

/* special kernel launch syntax */
#define __targetLaunch__(extent)
#define __targetLaunchNoStride__(extent)


/* Thread-level-parallelism execution macro */

#ifdef _OPENMP

#define __targetTLP__(simtIndex,extent)	\
_Pragma("omp parallel for")				\
for(simtIndex=0;simtIndex<extent;simtIndex+=VVL)

#define __targetTLPNoStride__(simtIndex,extent)   	\
_Pragma("omp parallel for")				\
for(simtIndex=0;simtIndex<extent;simtIndex++)

#else /* NOT OPENMP */

#define __targetTLP__(simtIndex,extent)	\
for(simtIndex=0;simtIndex<extent;simtIndex+=VVL)

#define __targetTLPNoStride__(simtIndex,extent)   	\
for(simtIndex=0;simtIndex<extent;simtIndex++)

#endif



/* Instruction-level-parallelism execution macro */
/* The __targetILP__ syntax is used, within a __targetTLP__ region, to specify
 * that the proceeding block of code should be executed in parallel and mapped to
 * instruction level parallelism (ILP), where the extent of the ILP is defined by the
 * virtual vector length (VVL) in the targetDP implementation. */
#if VVL == 1
#define __targetILP__(vecIndex) vecIndex = 0;
#else

#ifdef _OPENMP
#define __targetILP__(vecIndex)  \
_Pragma("omp simd")				\
 for (vecIndex = 0; vecIndex < VVL; vecIndex++) 
#else
#define __targetILP__(vecIndex)  \
 for (vecIndex = 0; vecIndex < VVL; vecIndex++) 
#endif

#endif

/* functions */

#define targetConstAddress(addr_of_ptr,const_object) \
  *addr_of_ptr=&(const_object);


#define copyConstToTarget(data_d, data, size) \
  memcpy(data_d,data,size);


#define copyConstFromTarget(data, data_d, size) \
  memcpy(data,data_d,size);


#endif



/* Common */

#define NILP VVL

/* Utility functions for indexing */

#define targetCoords3D(coords,extents,index)					\
  coords[0]=(index)/(extents[1]*extents[2]);				\
  coords[1] = ((index) - extents[1]*extents[2]*coords[0]) / extents[2];	\
  coords[2] = (index) - extents[1]*extents[2]*coords[0]			\
    - extents[2]*coords[1]; 

#define targetIndex3D(coords0,coords1,coords2,extents)	\
  extents[2]*extents[1]*(coords0)				\
  + extents[2]*(coords1)					\
  + (coords2); 

enum {TARGET_HALO,TARGET_EDGE};

/* API */
/* see specification or implementation for documentation on these */
__host__ void targetMalloc(void **address_of_ptr,const size_t size);
__host__ void targetCalloc(void **address_of_ptr,const size_t size);
__host__ void targetMallocUnified(void **address_of_ptr,const size_t size);
__host__ void targetCallocUnified(void **address_of_ptr,const size_t size);
__host__ void targetMallocHost(void **address_of_ptr,size_t size);
__host__ void copyToTarget(void *targetData,const void* data,size_t size);
__host__ void copyFromTarget(void *data,const void* targetData,size_t size);
__host__ void targetInit3D(int extents[3], size_t nfieldsmax, int nhalo);
__host__ void targetFinalize3D();
__host__ void targetInit(int extents[3], size_t nfieldsmax, int nhalo);
__host__ void targetFinalize();
__host__ void checkTargetError(const char *msg);

__host__ void copyToTargetMasked(double *targetData,const double* data,size_t nsites,
			size_t nfields,char* siteMask);
__host__ void copyFromTargetMasked(double *data,const double* targetData,size_t nsites,
			size_t nfields,char* siteMask);
__host__ void copyToTargetMaskedAoS(double *targetData,const double* data,size_t nsites,
			size_t nfields,char* siteMask);
__host__ void copyFromTargetMaskedAoS(double *data,const double* targetData,size_t nsites,
			size_t nfields,char* siteMask);

__host__ void copyFromTarget3DEdge(double *data,const double* targetData,int extents[3], size_t nfields);
__host__ void copyToTarget3DHalo(double *targetData,const double* data, int extents[3], size_t nfields);
__host__ void copyFromTargetPointerMap3D(double *data,const double* targetData, int extents[3], size_t nfields, int includeNeighbours, void** ptrarray);
__host__ void copyToTargetPointerMap3D(double *targetData,const double* data, int extents[3], size_t nfields, int includeNeighbours, void** ptrarray);
__host__ void copyFromTargetSubset(double *data,const double* targetData, int* sites, int nsitessubset, int nsites, int nfields);
__host__ void copyToTargetSubset(double *targetData,const double* data, int* sites, int nsitessubset, int nsites, int nfields);
__host__ void targetSynchronize();
__host__ void targetFree(void *ptr);
__host__ void checkTargetError(const char *msg);
__host__ void targetFree(void *ptr);
__host__ void targetZero(double* array,size_t size);
__host__ void targetSetConstant(double* array,double value,size_t size);
__host__ void targetAoS2SoA(double* array, size_t nsites, size_t nfields);
__host__ void targetSoA2AoS(double* array, size_t nsites, size_t nfields);


__host__ double targetDoubleSum(double* array, size_t size);

__host__ void copyDeepDoubleArrayToTarget(void* targetObjectAddress,void* hostObjectAddress,void* hostComponentAddress,int size);

__host__ void copyDeepDoubleArrayFromTarget(void* hostObjectAddress,void* targetObjectAddress,void* hostComponentAddress,int size);

#endif