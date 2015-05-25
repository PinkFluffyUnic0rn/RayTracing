#include "rt_funcs_render_pipe.h"

void rt_init( char *path )
{
	size_t sz = strlen( path );
	size_t szRt = strlen( "/CL/rt_raytrace.cl" );
	size_t tmpP = sz - 1;

	while ( path[tmpP] != '/' && tmpP != 0  )
		--tmpP;
	
	rt_cl_include_path = malloc( sizeof(char) * tmpP );
	rt_cl_raytrace_kernel_path = malloc( sizeof(char) * (tmpP + szRt) );

	strncpy( rt_cl_include_path, path, tmpP );
	strncpy( rt_cl_raytrace_kernel_path, path, tmpP );
	
	strcat( rt_cl_raytrace_kernel_path, "/CL/rt_raytrace.cl" );
}

void rt_render_pipe_create( rt_render_pipe *pRp, int w, int h )
{
	if ( !pRp )
		exit( -1 );

	rt_init_opencl( pRp );
	rt_init_buffers( pRp );

	pRp->screenData = NULL;
	rt_render_pipe_set_image_size( pRp, w, h );
}

void rt_render_pipe_calc_wg_size( rt_render_pipe *pRp, int w, int h )
{
	size_t maxSz;

	rt_opencl_content *ocl = &(pRp->oclContent);

	clGetDeviceInfo( ocl->dev, 
		CL_DEVICE_MAX_WORK_GROUP_SIZE, 
		sizeof(size_t), &maxSz, NULL );

	ocl->workGroupSz[0] = (size_t) sqrt( maxSz );
	ocl->workGroupSz[1] = (size_t) sqrt( maxSz );

	ocl->workGroupSz[0] *= ((float)pRp->w < (float)pRp->h) ? 
		((float)pRp->w / (float)pRp->h) : 1.0f; 
	ocl->workGroupSz[1] *= ((float)pRp->h < (float)pRp->w) ? 
		((float)pRp->h / (float)pRp->w) : 1.0f;


	if ( !(is_power_of_two_size_t(ocl->workGroupSz[0]) 
		&& is_power_of_two_size_t(ocl->workGroupSz[1])) )
	{
		size_t *minSz = NULL, *maxSz = NULL;
		size_t maxSzPos = 0, minSzPos = 0;
		size_t mask;

		if ( ocl->workGroupSz[0] > ocl->workGroupSz[1] )
		{
			minSz = ocl->workGroupSz + 1;
			maxSz = ocl->workGroupSz;
		}
		else
		{
			minSz = ocl->workGroupSz;
			maxSz = ocl->workGroupSz + 1;
		}

		*maxSz <<= 1;
			
		mask = (size_t)(1) << (8 * sizeof( size_t ) - 1);
			
		while ( !(*maxSz & mask) )
		{
			mask >>= 1;
			++maxSzPos;
		}
			
		mask = (size_t)(1) << (8 * sizeof( size_t ) - 1);
			
		while ( !(*minSz & mask) )
		{
			mask >>= 1;
			++minSzPos;
		}

		*minSz >>= 8 * sizeof(size_t) - minSzPos - 1;
		*minSz <<= 8 * sizeof(size_t) - minSzPos - 1;

		*maxSz >>= 8 * sizeof(size_t) - maxSzPos - 1;
		*maxSz <<= 8 * sizeof(size_t) - maxSzPos - 1;
	}	
}

void rt_render_pipe_set_image_size( rt_render_pipe *pRp, int w, int h )
{
	if ( !pRp )
		exit( -1 );

	rt_render_pipe_calc_wg_size( pRp, w, h );

	if ( pRp->screenData )
		free( pRp->screenData );

	pRp->screenData = (rt_argb *) malloc( sizeof(rt_argb) * h * w );
	pRp->w = w;
	pRp->h = h;
}

void rt_render_pipe_reset_blocks( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	clReleaseMemObject( pRp->memp );	
	clReleaseMemObject( pRp->mempdecs );
	clReleaseMemObject( pRp->meml );	
	clReleaseMemObject( pRp->memldecs );
	clReleaseMemObject( pRp->memt );
	clReleaseMemObject( pRp->memv );
	clReleaseMemObject( pRp->memm );

	rt_init_buffers( pRp );
}

void rt_init_buffers( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	pRp->primBlocks = 1;
	pRp->primDecsBlocks = 1;
	pRp->lightBlocks = 1;
	pRp->lightDecsBlocks = 1;
	pRp->triangleBlocks = 1;
	pRp->vertexBlocks = 1;
	pRp->materialBlocks = 1;

	pRp->mempdecs = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_prim_desc) * PRIMS_IN_BLOCK, NULL, NULL );
	pRp->memp = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		PRIMS_BLOCK_SIZE, NULL, NULL );
	pRp->primsEnd = 0;
	pRp->primsCount = 0;

	pRp->memldecs = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_light_desc) * LIGHTS_IN_BLOCK, NULL, NULL );
	pRp->meml = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		LIGHTS_BLOCK_SIZE, NULL, NULL );
	pRp->lightsEnd = 0;
	pRp->lightsCount = 0;

	pRp->memt = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_triangle)*TRIANGLES_IN_BLOCK, NULL, NULL );
	pRp->trianglesCount = 0;	

	pRp->memv = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_vertex)*VERTEX_IN_BLOCK, NULL, NULL );
	pRp->vertexCount = 0;	

	pRp->memm = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_material)*MATERIALS_IN_BLOCK, NULL, NULL );

	pRp->memc = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_camera), NULL, NULL );
}

void rt_render_pipe_set_camera( rt_render_pipe *pRp, 
	rt_camera *pFr )
{
	rt_camera *memPtr;
	
	if ( (pRp == NULL) || (pFr == NULL) )
		exit( -1 );

	memPtr = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memc, 
		CL_TRUE, CL_MAP_WRITE, 0, sizeof(rt_camera), 0, NULL, NULL, NULL );

	memcpy( memPtr, pFr, sizeof(rt_camera) );

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memc, memPtr, 0, 
		NULL, NULL );
}

void rt_add_block( rt_render_pipe *pRp, size_t blockSize, size_t nededSize,
	rt_ulong *curBlocksCount, cl_mem *buf )
{
	cl_mem tmp;

	if ( pRp == NULL )
		exit( -1 );

	rt_ulong newBlocksCount = nededSize / blockSize;
	newBlocksCount = newBlocksCount ? newBlocksCount : 1;
	
	tmp = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		blockSize * (*curBlocksCount + newBlocksCount), NULL, NULL );
	
	clEnqueueCopyBuffer( pRp->oclContent.commQue, *buf, tmp, 0, 0,
		blockSize * *curBlocksCount, 0, NULL, 0 );

	clReleaseMemObject( *buf );

	*buf = tmp;	
	*curBlocksCount += newBlocksCount;
}

void rt_render_pipe_add_primitive( rt_render_pipe *pRp, void *pPrim,
	RT_PRIMITIVE_TYPE pt )
{
	rt_prim_desc *pClPDecs;
	void *memPtr;
	
	if ( (pRp == NULL) || (pPrim == NULL) )
		exit( -1 );

	if ( (pRp->primsCount + 1) > PRIMS_IN_BLOCK * pRp->primDecsBlocks )
		rt_add_block( pRp, sizeof(rt_prim_desc) * PRIMS_IN_BLOCK,
			 sizeof(rt_prim_desc), &(pRp->primDecsBlocks), 
			 &(pRp->mempdecs) );
		
	pClPDecs = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->mempdecs, 
		CL_TRUE, CL_MAP_WRITE, 
		sizeof(rt_prim_desc) * (pRp->primsCount), 
		sizeof(rt_prim_desc), 0, NULL, NULL, NULL );

	pClPDecs->pt = pt;
	pClPDecs->offset = pRp->primsEnd;

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->mempdecs, pClPDecs, 0, 
		NULL, NULL );

	switch( pt )
	{
	case RT_PT_SPHERE:
		if ( (pRp->primsEnd + sizeof(rt_sphere)) 
				>= PRIMS_BLOCK_SIZE * pRp->primBlocks )
			rt_add_block( pRp, PRIMS_BLOCK_SIZE, sizeof(rt_sphere), 
				&(pRp->primBlocks),  &(pRp->memp) );

		memPtr = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memp, CL_TRUE, 
			CL_MAP_WRITE, pRp->primsEnd, sizeof(rt_sphere), 
			0, NULL, NULL, NULL );
		
		memcpy( memPtr, pPrim, sizeof(rt_sphere) );

		clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memp, memPtr, 0, 
			NULL, NULL );
	
		pRp->primsEnd += sizeof(rt_sphere);
		break;

	default:
		break;
	}
	
	++(pRp->primsCount);
}

void rt_render_pipe_add_triangles( rt_render_pipe *pRp, 
	rt_vertex *pV, rt_triangle *pTr, rt_ulong vCount, rt_ulong tCount )
{
	rt_triangle *pTrM;
	int i;

	if ( (pRp == NULL) || (pV == NULL) || (pTr == NULL) )
		exit( -1 );
	
	if ( (pRp->trianglesCount + tCount)
			>= TRIANGLES_IN_BLOCK * pRp->triangleBlocks )
		rt_add_block( pRp, TRIANGLES_IN_BLOCK * sizeof(rt_triangle), 
			sizeof(rt_triangle) * tCount,  &(pRp->triangleBlocks),
			&(pRp->memt) );
	
	if ( (pRp->vertexCount + vCount) >= VERTEX_IN_BLOCK
		* pRp->vertexBlocks)
		rt_add_block( pRp, VERTEX_IN_BLOCK * sizeof(rt_vertex), 
			sizeof(rt_vertex) * vCount,  &(pRp->vertexBlocks),
			&(pRp->memv) );
	
	clEnqueueWriteBuffer( pRp->oclContent.commQue, pRp->memv, 
		CL_TRUE, sizeof(rt_vertex) * (pRp->vertexCount), 
		sizeof(rt_vertex) * vCount, pV, 0, NULL, NULL );

	
	pTrM = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memt, CL_TRUE,
		CL_MAP_WRITE, sizeof(rt_triangle) * (pRp->trianglesCount),
		sizeof(rt_triangle) * tCount, 0, NULL, NULL, NULL );
	
	for ( i = 0; i < tCount; ++i )
	{
		pTrM[i].pV0 = pRp->vertexCount + pTr[i].pV0;
		pTrM[i].pV1 = pRp->vertexCount + pTr[i].pV1;
		pTrM[i].pV2 = pRp->vertexCount + pTr[i].pV2;
		pTrM[i].mat = pTr[i].mat;

	}

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memt, pTrM, 0, 
		NULL, NULL );

	pRp->trianglesCount += tCount;
	pRp->vertexCount += vCount;
}

void rt_render_pipe_add_material( rt_render_pipe *pRp, rt_material *pMat, 
	unsigned long int ind )
{
	if ( (pRp == NULL) || (pMat == NULL) )
		exit( -1 );

	if ( (ind < 0) )
		exit( -2 );

	if ( ind >= MATERIALS_IN_BLOCK * pRp->materialBlocks )
		rt_add_block( pRp, MATERIALS_IN_BLOCK * sizeof(rt_material), 
			sizeof(rt_material),  &(pRp->materialBlocks),
			&(pRp->memm) );

	clEnqueueWriteBuffer( pRp->oclContent.commQue, pRp->memm, 
		CL_TRUE, sizeof(rt_material) * ind, 
		sizeof(rt_material), pMat, 0, NULL, NULL );
}

void rt_render_pipe_add_light( rt_render_pipe *pRp, void *pLight,
	RT_LIGHT_TYPE lt )
{
	int err;
	rt_light_desc *pClLDecs;
	void *memPtr;

	if ( (pRp == NULL) || (pLight == NULL) )
		exit( -1 );

	if ( (pRp->lightsCount + 1) > LIGHTS_IN_BLOCK * pRp->lightDecsBlocks )
		rt_add_block( pRp, sizeof(rt_light_desc) * LIGHTS_IN_BLOCK,
			 sizeof(rt_light_desc), &(pRp->lightDecsBlocks), 
			 &(pRp->memldecs) );

	pClLDecs = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memldecs, 
		CL_TRUE, CL_MAP_WRITE, 
		sizeof(rt_light_desc) * (pRp->lightsCount), 
		sizeof(rt_light_desc), 0, NULL, NULL, NULL );

	pClLDecs->lt = lt;
	pClLDecs->offset = pRp->lightsEnd;

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memldecs, pClLDecs, 0, 
		NULL, NULL );

	switch( lt )
	{
	case RT_LT_POINT:	
		if ( (pRp->lightsEnd + sizeof(rt_point_light))
			>= LIGHTS_BLOCK_SIZE * pRp->lightBlocks )
			rt_add_block( pRp, LIGHTS_BLOCK_SIZE, 
				sizeof(rt_point_light), &(pRp->lightBlocks),
				&(pRp->meml) );

		memPtr = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->meml, CL_TRUE, 
			CL_MAP_WRITE, pRp->lightsEnd, sizeof(rt_point_light), 
			0, NULL, NULL, &err );
		memcpy( memPtr, pLight, sizeof(rt_point_light) );

		clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->meml, memPtr, 0, 
			NULL, NULL );
	
		pRp->lightsEnd += sizeof(rt_point_light);
		break;

	default:
		break;
	}
		
	++(pRp->lightsCount);
}

void rt_render_pipe_get_camera( rt_render_pipe *pRp, rt_camera **ppCam )
{
	if ( pRp == NULL || ppCam == NULL )
		exit(-1);

	*ppCam = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memc, 
		CL_TRUE, CL_MAP_WRITE | CL_MAP_READ, 0, sizeof(rt_camera), 0,
		NULL, NULL, NULL );
}

void rt_render_pipe_free_camera( rt_render_pipe *pRp, rt_camera *pC )
{
	if ( pRp == NULL || pC == NULL )
		exit( -1 );

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memc, pC, 0, 
		NULL, NULL );
}

void rt_init_opencl( rt_render_pipe *pRp )
{
	cl_platform_id plID[5];
	cl_uint plCount;
	cl_device_id devID[5][5];
	cl_uint devCount[5];
	cl_program prog;

	int i;
	cl_int ret;
		
	size_t progsrcsz;
	FILE *fp;
	char *progsrc;
	
	char *clArgs = malloc( sizeof(char) 
		* ( strlen(rt_cl_include_path) + 4) );

	if ( pRp == NULL )
		exit( -1 );

	rt_opencl_content *ocl = &(pRp->oclContent);

	ret = clGetPlatformIDs( 5, plID, &(plCount) );	
	for ( i = 0; i < plCount; ++i )
		clGetDeviceIDs( plID[i], CL_DEVICE_TYPE_GPU, 5, 
			devID[i], devCount+i );

	ocl->dev = devID[0][0];

	{
		char tmp[1024];

		clGetPlatformInfo( plID[0], CL_PLATFORM_NAME, 
			1024, tmp, NULL );
		printf( "choosen platform: %s\n", tmp );
		
		clGetDeviceInfo( devID[0][0], CL_DEVICE_NAME,
			1024, tmp, NULL );
		
		printf( "choosen device: %s\n", tmp );
	}
	
	ocl->context = clCreateContext( NULL, 1, devID[0], NULL, 
		NULL, &ret );
	ocl->commQue = clCreateCommandQueue( ocl->context, 
		devID[0][0], 0, &ret );

	
	fp = fopen( rt_cl_raytrace_kernel_path, "r" );
	progsrc = (char *) malloc(0x10000000);

	if ( fp == NULL )
	{
		printf( "open file error\n" );
		exit( -3 );
	}
		
	progsrcsz = fread( progsrc, sizeof(char), 0x10000000, fp );
	fclose( fp );

	prog = clCreateProgramWithSource( ocl->context, 1, 
		(const char **)(&progsrc), 
		(const size_t *)(&progsrcsz), &ret );

	if ( ret != CL_SUCCESS )
	{
		printf( "clCreateProgramWithSource fail\n" );
		exit( -3 );
	}

	strcpy( clArgs, "-I " );
	strcat( clArgs, rt_cl_include_path );
	
	ret = clBuildProgram( prog, 1, devID[0],
		clArgs, NULL, NULL );

	if ( ret != CL_SUCCESS )
	{
	    	size_t log_size;
   		char *log;
    			
		clGetProgramBuildInfo( prog, devID[0][0],
			CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size );

   		log = (char *) malloc(log_size);

    		clGetProgramBuildInfo( prog, devID[0][0],
			CL_PROGRAM_BUILD_LOG, log_size, log, NULL );

		printf( "clBuildProgram fail:\n" );
    		printf("%s\n", log);
			
		exit( -3 );
	}
	
	ocl->raytrace = clCreateKernel( prog, "raytrace", &ret );
}

void rt_opencl_render( rt_render_pipe *pRp, rt_box *pBoundingBox,
	cl_mem *memi, cl_mem *memn )
{
	rt_raytrace_args *rtArgs;
	rt_opencl_content *pOcl = &(pRp->oclContent);
	cl_mem mema;
	cl_mem memout;

	if ( pRp == NULL )
		exit( -1 );

	// first arg
	mema = clCreateBuffer( pOcl->context, CL_MEM_READ_ONLY, 
		sizeof(rt_raytrace_args), NULL, NULL );

	rtArgs = clEnqueueMapBuffer( pOcl->commQue, mema, CL_TRUE, CL_MAP_WRITE, 0, 
		sizeof(rt_raytrace_args), 0, NULL, NULL, NULL );
	
	rtArgs->primsCount = pRp->primsCount;
	rtArgs->trianglesCount = pRp->trianglesCount;
	rtArgs->vertexCount = pRp->vertexCount;
	rtArgs->lightsCount = pRp->lightsCount;
	rtArgs->w = pRp->w;
	rtArgs->h = pRp->h;
	rtArgs->xdelta = 1;
	rtArgs->ydelta = 1;

	rtArgs->boundingBox = *pBoundingBox;

	clEnqueueUnmapMemObject( pOcl->commQue, mema, rtArgs, 0, 
		NULL, NULL );

	// output arg
	memout = clCreateBuffer( pOcl->context, CL_MEM_WRITE_ONLY, 
		sizeof(rt_argb)*(pRp->w)*(pRp->h), NULL, NULL );
	
	// enqueue threads
	{
		size_t gwSz[2] = { pRp->w + ( 32 - (pRp->w % 32) ),
			pRp->h + ( 32 - (pRp->h % 32) ) };
		
		clSetKernelArg( pOcl->raytrace, 0, 
			sizeof(cl_mem), (void *) &mema );
		clSetKernelArg( pOcl->raytrace, 1, 
			sizeof(cl_mem), (void *) &(pRp->memp) );
		clSetKernelArg( pOcl->raytrace, 2, 
			sizeof(cl_mem), (void *) &(pRp->mempdecs) );
		clSetKernelArg( pOcl->raytrace, 3, 
			sizeof(cl_mem), (void *) &(pRp->meml) );
		clSetKernelArg( pOcl->raytrace, 4, 
			sizeof(cl_mem), (void *) &(pRp->memldecs) );
		clSetKernelArg( pOcl->raytrace, 5,
			sizeof(cl_mem), (void *) &(pRp->memc) );
		clSetKernelArg( pOcl->raytrace, 6,
			sizeof(cl_mem), (void *) &(pRp->memm) );
		clSetKernelArg( pOcl->raytrace, 7,
			sizeof(cl_mem), (void *) &(pRp->memt) );
		clSetKernelArg( pOcl->raytrace, 8,
			sizeof(cl_mem), (void *) &(pRp->memv) );
		clSetKernelArg( pOcl->raytrace, 9,
			sizeof(cl_mem), (void *) memn );
		clSetKernelArg( pOcl->raytrace, 10,
			sizeof(cl_mem), (void *) memi );
		clSetKernelArg( pOcl->raytrace, 11,
			sizeof(cl_mem), (void *) &memout );

		clEnqueueNDRangeKernel( pOcl->commQue, pOcl->raytrace, 2, 
			NULL, gwSz, pOcl->workGroupSz, 0, NULL, NULL );	
	}
	
	clEnqueueReadBuffer( pOcl->commQue, memout, CL_TRUE, 0, 
		(pRp->w) * (pRp->h) * sizeof(rt_argb), pRp->screenData, 
		0, NULL, NULL );

	clReleaseMemObject( mema );	
	clReleaseMemObject( memout );
}

void rt_render_pipe_cleanup( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	free( pRp->screenData );

	clReleaseMemObject( pRp->memp );	
	clReleaseMemObject( pRp->mempdecs );
	clReleaseMemObject( pRp->meml );	
	clReleaseMemObject( pRp->memldecs );
	clReleaseMemObject( pRp->memt );
	clReleaseMemObject( pRp->memv );
	clReleaseMemObject( pRp->memm );
}

void *rt_compute_sah_help_thread( void *pArgs )
{
	rt_compute_sah_help_args *args = (rt_compute_sah_help_args *)pArgs;
	rt_compute_sah_help_return *rets = malloc(sizeof(rt_compute_sah_help_return));
	int end = SAH_PARTS / COMPUTE_SAH_THREADS * args->threadId + SAH_PARTS / COMPUTE_SAH_THREADS;	
	int i;
	int b = 1;
	float extents_xy_area = args->pBox->extents.x * args->pBox->extents.y,
		extents_yz_area = args->pBox->extents.y * args->pBox->extents.z,
		extents_xz_area = args->pBox->extents.x * args->pBox->extents.z;

	rt_vertex *memVer = args->memVer;

	if ( pArgs == NULL )
		exit( -1 );

	switch ( args->axis )
	{
	case RT_AXIS_X:	
		for ( i = SAH_PARTS / COMPUTE_SAH_THREADS * args->threadId; i < end; ++i )
		{
			float sep = args->min + i * args->delta;
			float sah;
			int j;
			rt_ulong tmpPrimsCountL = 0;
			rt_ulong tmpPrimsCountR = 0;
		
			for ( j = 0; j < args->primsCount; ++j )
			{		
				float maxV, minV;
				rt_triangle *tmpTr = args->memTr + (args->memTp)[j];

				maxV = memVer[tmpTr->pV0].pos.x;	
				if ( maxV < memVer[tmpTr->pV1].pos.x )
					maxV = memVer[tmpTr->pV1].pos.x;
				if ( maxV < memVer[tmpTr->pV2].pos.x )
					maxV = memVer[tmpTr->pV2].pos.x;

				minV = memVer[tmpTr->pV0].pos.x;
				if ( minV > memVer[tmpTr->pV1].pos.x )
					minV = memVer[tmpTr->pV1].pos.x;
				if ( minV > memVer[tmpTr->pV2].pos.x )
					minV = memVer[tmpTr->pV2].pos.x;

				if ( minV < sep )
					++tmpPrimsCountL;
				if ( maxV > sep )
					++tmpPrimsCountR;
			}

			sah = ( extents_yz_area
				+ (sep - args->pBox->center.x + args->pBox->extents.x)
				* (args->pBox->extents.y + args->pBox->extents.z) )
				* tmpPrimsCountL
				+ 
				( extents_yz_area
				+ (args->pBox->center.x + args->pBox->extents.x - sep)
				* (args->pBox->extents.y + args->pBox->extents.z) )
				* tmpPrimsCountR;

			
			if ( b || sah < rets->resSAH )
			{
				rets->resSAH = sah;
				rets->resSep = sep;
				rets->resPrimsL = tmpPrimsCountL;
				rets->resPrimsR = tmpPrimsCountR;
				b = 0;
			}
		}
		break;
	
	case RT_AXIS_Y:
		for ( i = SAH_PARTS / COMPUTE_SAH_THREADS * args->threadId; i < end; ++i )
		{		
			float sep = args->min + i * args->delta;
			float sah;
			int j;
			rt_ulong tmpPrimsCountL = 0;
			rt_ulong tmpPrimsCountR = 0;
			
			for ( j = 0; j < args->primsCount; ++j )
			{
				float maxV, minV;
				rt_triangle *tmpTr = args->memTr + (args->memTp)[j];

				maxV = memVer[tmpTr->pV0].pos.y;	
				if ( maxV < memVer[tmpTr->pV1].pos.y )
					maxV = memVer[tmpTr->pV1].pos.y;
				if ( maxV < memVer[tmpTr->pV2].pos.y )
					maxV = memVer[tmpTr->pV2].pos.y;

				minV = memVer[tmpTr->pV0].pos.y;
				if ( minV > memVer[tmpTr->pV1].pos.y )
					minV = memVer[tmpTr->pV1].pos.y;
				if ( minV > memVer[tmpTr->pV2].pos.y )
					minV = memVer[tmpTr->pV2].pos.y;
	
				if ( minV < sep )
					++tmpPrimsCountL;
				if ( maxV > sep )
					++tmpPrimsCountR;
			}

			sah = ( extents_xz_area
				+ (sep - args->pBox->center.y + args->pBox->extents.y)
				* (args->pBox->extents.x + args->pBox->extents.z) )
				* tmpPrimsCountL
				+ 
				( extents_xz_area
				+ (args->pBox->center.y + args->pBox->extents.y - sep)
				* (args->pBox->extents.x + args->pBox->extents.z) )
				* tmpPrimsCountR;
					
			if ( b || sah < rets->resSAH )
			{
				rets->resSAH = sah;
				rets->resSep = sep;
				rets->resPrimsL = tmpPrimsCountL;
				rets->resPrimsR = tmpPrimsCountR;
				b = 0;
			}
		}
		break;

	case RT_AXIS_Z:
		for ( i = SAH_PARTS / COMPUTE_SAH_THREADS * args->threadId; i < end; ++i )
		{
			float sep = args->min + i * args->delta;
			float sah;
			int j;
			rt_ulong tmpPrimsCountL = 0;
			rt_ulong tmpPrimsCountR = 0;

			for ( j = 0; j < args->primsCount; ++j )
			{
				float maxV, minV;
				rt_triangle *tmpTr = args->memTr + (args->memTp)[j];

				maxV = memVer[tmpTr->pV0].pos.z;	
				if ( maxV < memVer[tmpTr->pV1].pos.z )
					maxV = memVer[tmpTr->pV1].pos.z;
				if ( maxV < memVer[tmpTr->pV2].pos.z )
					maxV = memVer[tmpTr->pV2].pos.z;

				minV = memVer[tmpTr->pV0].pos.z;
				if ( minV > memVer[tmpTr->pV1].pos.z )
					minV = memVer[tmpTr->pV1].pos.z;
				if ( minV > memVer[tmpTr->pV2].pos.z )
					minV = memVer[tmpTr->pV2].pos.z;
			
				if ( minV < sep )
					++tmpPrimsCountL;
				if ( maxV > sep )
					++tmpPrimsCountR;
			}

			sah = ( extents_xy_area
				+ (sep - args->pBox->center.z + args->pBox->extents.z)
				* (args->pBox->extents.x + args->pBox->extents.y) )
				* tmpPrimsCountL
				+ 
				( extents_xy_area
				+ (args->pBox->center.z + args->pBox->extents.z - sep)
				* (args->pBox->extents.x + args->pBox->extents.y) )
				* tmpPrimsCountR;
						
			if ( b || sah < rets->resSAH )
			{
				rets->resSAH = sah;
				rets->resSep = sep;
				rets->resPrimsL = tmpPrimsCountL;
				rets->resPrimsR = tmpPrimsCountR;
				b = 0;
			}

		}
		break;
	}

	return rets;
}

void rt_kdtree_compute_sah( rt_ulong *memTp, rt_triangle *memTr, 
	rt_vertex *memVer, rt_ulong primsCount, RT_AXIS axis, 
	rt_box *pBox, rt_ulong *primsCountL, rt_ulong *primsCountR, 
	float *pSep )
{
	float min, max;
	float delta;
	int i;
	
	if ( (memTp == NULL) || (memTr == NULL) || (memVer == NULL)
		|| (pBox == NULL) || (primsCountL == NULL)
		|| (primsCountR == NULL) || (pSep == NULL) )
		exit( -1 );

	switch ( axis )
	{
	case RT_AXIS_X:
		min = pBox->center.x - pBox->extents.x;
		max = pBox->center.x + pBox->extents.x;
		break;
	
	case RT_AXIS_Y:
		min = pBox->center.y - pBox->extents.y;
		max = pBox->center.y + pBox->extents.y;
		break;
	
	case RT_AXIS_Z:
		min = pBox->center.z - pBox->extents.z;
		max = pBox->center.z + pBox->extents.z;
		break;
	}

	delta = (max - min) / (float) SAH_PARTS;

	{
		pthread_t *thrs = (pthread_t *) malloc( sizeof(pthread_t) * COMPUTE_SAH_THREADS );
		rt_compute_sah_help_args *prms = (rt_compute_sah_help_args *) malloc( 
			sizeof(rt_compute_sah_help_args) * COMPUTE_SAH_THREADS );
		rt_compute_sah_help_return **rets = (rt_compute_sah_help_return **) malloc( 
			sizeof(rt_compute_sah_help_return *) * COMPUTE_SAH_THREADS );
		float minSAH;
		int minID;


		for ( i = 0; i < COMPUTE_SAH_THREADS; ++i ) 
		{
			prms[i].pBox = pBox;
			prms[i].memTp = memTp;
			prms[i].memTr = memTr;
			prms[i].memVer = memVer;
			prms[i].primsCount = primsCount;
			prms[i].min = min;
			prms[i].delta = delta;
			prms[i].threadId = i;
			prms[i].axis = axis;
		
			pthread_create( thrs + i, NULL, &rt_compute_sah_help_thread, 
				(void *)(prms + i) );
		}
	
		for ( i = 0; i < COMPUTE_SAH_THREADS; ++i )
			pthread_join( thrs[i], (void *)(rets + i) );
	
		minID = 0;
		minSAH = rets[0]->resSAH;
		for ( i = 0; i < COMPUTE_SAH_THREADS; ++i )
			if ( rets[i]->resSAH < minSAH )
			{
				minID = i;
				minSAH = rets[i]->resSAH;
			}
		
		*primsCountL = rets[minID]->resPrimsL;
		*primsCountR = rets[minID]->resPrimsR;
		*pSep = rets[minID]->resSep;

		free(thrs);
		free(prms);

		for ( i = 0; i < COMPUTE_SAH_THREADS; ++i )
			free(rets[i]);
		free(rets);
	}
}

//with destruction
rt_kdtree_count_info rt_kdtree_pack_to_buffer( rt_cl_kdtree_node *pNodeBuf, 
	rt_ulong *pPrimsIdxBuf, rt_kdtree_node *pNode, 
	rt_ulong writePos, rt_ulong primsWritePos )
{
	rt_kdtree_count_info leftChilds, rightChilds;

	if ( (pNodeBuf == NULL) || (pPrimsIdxBuf == NULL) 
		|| (pNode == NULL) )
		exit( -1 );

	pNodeBuf[writePos].isLast = pNode->isLast;

	if ( pNode->isLast )
	{
		rt_kdtree_count_info tmpCntI;
		
		tmpCntI.nodesCount = 1;
		tmpCntI.primsCount = pNode->primsCount;

		pNodeBuf[writePos].primsCount = pNode->primsCount;
		pNodeBuf[writePos].prims = primsWritePos;
		
		memcpy( pPrimsIdxBuf + primsWritePos, pNode->prims, 
			sizeof(rt_ulong) * pNode->primsCount );
	
		free( pNode->prims );
	
		return tmpCntI;
	}
		
	leftChilds = rt_kdtree_pack_to_buffer( pNodeBuf, pPrimsIdxBuf, 
		pNode->leftNode, writePos + 1, primsWritePos );
	rightChilds = rt_kdtree_pack_to_buffer( pNodeBuf, pPrimsIdxBuf, 
		pNode->rightNode, writePos + leftChilds.nodesCount + 1, 
		primsWritePos + leftChilds.primsCount );

	pNodeBuf[writePos].sep = pNode->sep;
	pNodeBuf[writePos].axis = pNode->axis;
	pNodeBuf[writePos].leftNode = writePos + 1;
	pNodeBuf[writePos].rightNode = writePos + leftChilds.nodesCount + 1;

	free( pNode );

	{
		rt_kdtree_count_info tmpCntI;

		tmpCntI.nodesCount = leftChilds.nodesCount
			+ rightChilds.nodesCount + 1;
		tmpCntI.primsCount = leftChilds.primsCount
			+ rightChilds.primsCount;
		
		return tmpCntI;
	}
}

rt_kdtree_count_info rt_kdtree_make_childs( rt_vertex *memVer,
	rt_triangle *memTr, rt_kdtree_node *pNode, rt_box *bbox, int depth )
{
	float range = bbox->extents.x;
	rt_ulong i, j, k;
	rt_box tmpBoxL, tmpBoxR;
	
	if ( (memVer == NULL) || (memTr == NULL)
		|| (pNode == NULL) || (bbox == NULL) )
		exit( -1 );

	// cheking conditions for end of recursion
	if ( pNode->primsCount <= MAX_PRIMS_IN_NODE 
		|| depth == MAX_DEPTH )
	{
		rt_kdtree_count_info tmpCntI;

		pNode->isLast = 1;
		pNode->leftNode = NULL;	
		pNode->rightNode = NULL;
		
		tmpCntI.primsCount = pNode->primsCount;
		tmpCntI.nodesCount = 1;

		return tmpCntI;
	}
	else
		pNode->isLast = 0;

	
	// choosing axis for separation
	pNode->axis = RT_AXIS_X;

	if ( range < bbox->extents.y )
	{
		range = bbox->extents.y;
		pNode->axis = RT_AXIS_Y;
	}	
	
	if ( range < bbox->extents.z )
	{
		range = bbox->extents.z;
		pNode->axis = RT_AXIS_Z;
	}	

	// computing SAH
	pNode->leftNode = malloc( sizeof(rt_kdtree_node) );
	pNode->rightNode = malloc( sizeof(rt_kdtree_node) );
	
	rt_kdtree_compute_sah( pNode->prims, memTr, memVer, pNode->primsCount, 
		pNode->axis, bbox, &(pNode->leftNode->primsCount), 
		&(pNode->rightNode->primsCount), &(pNode->sep) );

	pNode->leftNode->prims = malloc( sizeof(rt_ulong) * pNode->leftNode->primsCount );
	pNode->rightNode->prims = malloc( sizeof(rt_ulong) * pNode->rightNode->primsCount );

	// place prims into node
	j = k = 0;

	for ( i = 0; i < pNode->primsCount; ++i )
	{
		float maxV, minV;
		rt_triangle *tmpTr = memTr + pNode->prims[i];

		switch ( pNode->axis )
		{
		case RT_AXIS_X:
			maxV = memVer[tmpTr->pV0].pos.x;	
			if ( maxV < memVer[tmpTr->pV1].pos.x )
				maxV = memVer[tmpTr->pV1].pos.x;
			if ( maxV < memVer[tmpTr->pV2].pos.x )
				maxV = memVer[tmpTr->pV2].pos.x;
		
			minV = memVer[tmpTr->pV0].pos.x;
			if ( minV > memVer[tmpTr->pV1].pos.x )
				minV = memVer[tmpTr->pV1].pos.x;
			if ( minV > memVer[tmpTr->pV2].pos.x )
				minV = memVer[tmpTr->pV2].pos.x;
			break;

		case RT_AXIS_Y:
			maxV = memVer[tmpTr->pV0].pos.y;	
			if ( maxV < memVer[tmpTr->pV1].pos.y )
				maxV = memVer[tmpTr->pV1].pos.y;
			if ( maxV < memVer[tmpTr->pV2].pos.y )
				maxV = memVer[tmpTr->pV2].pos.y;
		
			minV = memVer[tmpTr->pV0].pos.y;
			if ( minV > memVer[tmpTr->pV1].pos.y )
				minV = memVer[tmpTr->pV1].pos.y;
			if ( minV > memVer[tmpTr->pV2].pos.y )
				minV = memVer[tmpTr->pV2].pos.y;
			break;
			
		case RT_AXIS_Z:
			maxV = memVer[tmpTr->pV0].pos.z;	
			if ( maxV < memVer[tmpTr->pV1].pos.z )
				maxV = memVer[tmpTr->pV1].pos.z;
			if ( maxV < memVer[tmpTr->pV2].pos.z )
				maxV = memVer[tmpTr->pV2].pos.z;
		
			minV = memVer[tmpTr->pV0].pos.z;
			if ( minV > memVer[tmpTr->pV1].pos.z )
				minV = memVer[tmpTr->pV1].pos.z;
			if ( minV > memVer[tmpTr->pV2].pos.z )
				minV = memVer[tmpTr->pV2].pos.z;
			break;
		}


		if ( minV < pNode->sep )
			pNode->leftNode->prims[j++] = pNode->prims[i];
		if ( maxV > pNode->sep )
			pNode->rightNode->prims[k++] = pNode->prims[i];

	}
	
	free( pNode->prims );

	// making bounding box for child nodes
	tmpBoxR = tmpBoxL = *bbox;

	switch ( pNode->axis )
	{
	case RT_AXIS_X:
		tmpBoxL.center.x = 0.5f * ( bbox->center.x
			- bbox->extents.x + pNode->sep );
		tmpBoxL.extents.x = 0.5f * ( pNode->sep
			- bbox->center.x + bbox->extents.x );

		tmpBoxR.center.x = 0.5f * ( bbox->center.x
			+ bbox->extents.x + pNode->sep );
		tmpBoxR.extents.x = 0.5f * ( bbox->center.x
			+ bbox->extents.x- pNode->sep );
		break;

	case RT_AXIS_Y:
		tmpBoxL.center.y = 0.5f * ( bbox->center.y
			- bbox->extents.y + pNode->sep );
		tmpBoxL.extents.y = 0.5f * ( pNode->sep 
			- bbox->center.y + bbox->extents.y );

		tmpBoxR.center.y = 0.5f * ( bbox->center.y
			+ bbox->extents.y + pNode->sep );
		tmpBoxR.extents.y = 0.5f * ( bbox->center.y
			+ bbox->extents.y - pNode->sep );
		break;

	case RT_AXIS_Z:
		tmpBoxL.center.z = 0.5f * ( bbox->center.z
			- bbox->extents.z + pNode->sep );
		tmpBoxL.extents.z = 0.5f * ( pNode->sep 
			- bbox->center.z + bbox->extents.z );

		tmpBoxR.center.z = 0.5f * ( bbox->center.z
			+ bbox->extents.z + pNode->sep );
		tmpBoxR.extents.z = 0.5f * ( bbox->center.z
			+ bbox->extents.z - pNode->sep );
		break;
	}
	
	// recursive call
	{
		rt_kdtree_count_info tmpCntI, cntL, cntR;

		cntL = rt_kdtree_make_childs( memVer, memTr, pNode->leftNode, 
			&tmpBoxL, depth + 1 );
		cntR = rt_kdtree_make_childs( memVer, memTr, pNode->rightNode, 
			&tmpBoxR, depth + 1 );
	
		tmpCntI.primsCount = cntL.primsCount + cntR.primsCount;
		tmpCntI.nodesCount = cntL.nodesCount + cntR.nodesCount + 1;

		return tmpCntI;
	}
}

void rt_kdtree_build( rt_render_pipe *pRp, rt_box *pBoundingBox,
	cl_mem *memi, cl_mem *memn )
{
	rt_triangle *memTr = malloc( sizeof(rt_triangle) * pRp->trianglesCount );
	rt_vertex *memVer = malloc( sizeof(rt_vertex) * pRp->vertexCount );
	rt_ulong i;
	rt_vector3 minP, maxP;
	rt_ulong primsInNodesCount;
	rt_ulong nodesCount;
	rt_kdtree_node *rootNode = malloc( sizeof(rt_kdtree_node) );

	if ( pRp == NULL )
		exit( -1 );

	clEnqueueReadBuffer( pRp->oclContent.commQue, pRp->memt, CL_TRUE, 0, 
		sizeof(rt_vertex) * pRp->trianglesCount, memTr, 
		0, NULL, NULL );

	clEnqueueReadBuffer( pRp->oclContent.commQue, pRp->memv, CL_TRUE, 0, 
		sizeof(rt_vertex) * pRp->vertexCount, memVer, 
		0, NULL, NULL );

	// making bounding box
	for ( i = 0; i < pRp->vertexCount; ++i )
	{
		minP.x = (minP.x > memVer[i].pos.x || i == 0) ? memVer[i].pos.x : minP.x;
		minP.y = (minP.y > memVer[i].pos.y || i == 0) ? memVer[i].pos.y : minP.y;
		minP.z = (minP.z > memVer[i].pos.z || i == 0) ? memVer[i].pos.z : minP.z;
	

		maxP.x = (maxP.x < memVer[i].pos.x || i == 0) ? memVer[i].pos.x : maxP.x;
		maxP.y = (maxP.y < memVer[i].pos.y || i == 0) ? memVer[i].pos.y : maxP.y;
		maxP.z = (maxP.z < memVer[i].pos.z || i == 0) ? memVer[i].pos.z : maxP.z;
	}

	minP.x -= 1.0f; minP.y -= 1.0f; minP.z -= 1.0f;
	maxP.x += 1.0f; maxP.y += 1.0f; maxP.z += 1.0f;

	rt_vector3_add( &maxP, &minP, &(pBoundingBox->center) );
	rt_vector3_scalar_mult( &(pBoundingBox->center), 0.5f, 
		&(pBoundingBox->center) );

	rt_vector3_sub( &minP, &maxP, &(pBoundingBox->extents) );
	rt_vector3_scalar_mult( &(pBoundingBox->extents), 0.5f, 
		&(pBoundingBox->extents) );

	// creating nodes
	rootNode->prims = malloc( sizeof(rt_triangle) * pRp->trianglesCount );	
	rootNode->primsCount = pRp->trianglesCount;
	
	for ( i = 0; i < pRp->trianglesCount; ++i )
		rootNode->prims[i] = i;

	if ( pRp->trianglesCount < MAX_PRIMS_IN_NODE )
	{
		rootNode->isLast = 1;
	
		primsInNodesCount = pRp->trianglesCount;
		nodesCount = 1;
	}
	else
	{
		rt_kdtree_count_info tmpCntI;

		rootNode->isLast = 0;

		tmpCntI = rt_kdtree_make_childs( memVer, memTr, 
			rootNode, pBoundingBox, 0 );

		primsInNodesCount = tmpCntI.primsCount;
		nodesCount = tmpCntI.nodesCount;
	}

	free( memTr );
	memTr = NULL;
	free( memVer );
	memVer = NULL;

	// packing nodes into OpenCL buffers 
	{
		rt_cl_kdtree_node *memNode;
		rt_ulong *memTrIdx;

		*memi = clCreateBuffer( pRp->oclContent.context, 
			CL_MEM_READ_ONLY, sizeof(rt_ulong) * primsInNodesCount, 
			NULL, NULL );
		*memn = clCreateBuffer( pRp->oclContent.context, 
			CL_MEM_READ_ONLY, 
			sizeof(rt_cl_kdtree_node) * nodesCount, NULL, NULL );

		memNode = clEnqueueMapBuffer( pRp->oclContent.commQue, 
			*memn, CL_TRUE, CL_MAP_WRITE, 0, 
			sizeof(rt_cl_kdtree_node) * nodesCount, 
			0, NULL, NULL, NULL );
		memTrIdx = clEnqueueMapBuffer( pRp->oclContent.commQue, 
			*memi, CL_TRUE, CL_MAP_WRITE, 0, 
			sizeof(rt_ulong) * primsInNodesCount, 
			0, NULL, NULL, NULL );

		/*
		rt_dout_kdtree( memVer, memTr, pRp, rootNode, 0 );
		exit(1);
		*/
		
		rt_kdtree_pack_to_buffer( memNode, 
			memTrIdx, rootNode, 0, 0 );
/*	
		rt_cl_dout_kdtree( memVer, memTr, memNode, memTrIdx, pRp, 0, 0 );
		exit(1);
*/
		clEnqueueUnmapMemObject( pRp->oclContent.commQue, *memn, 
			memNode, 0, NULL, NULL );
	 	clEnqueueUnmapMemObject( pRp->oclContent.commQue, *memi, 
			memTrIdx, 0, NULL, NULL );
	}

}

rt_argb *rt_render_pipe_draw( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	rt_box bbox;
	cl_mem memi;
	cl_mem memn;

	rt_kdtree_build( pRp, &bbox, &memi, &memn );
	rt_opencl_render( pRp, &bbox, &memi, &memn  );

	clReleaseMemObject( memi );
	clReleaseMemObject( memn );

	pRp->trianglesCount = 0;
	pRp->vertexCount = 0;
	pRp->lightsCount = 0;
	pRp->primsCount = 0;
	pRp->primsEnd = 0;
	pRp->lightsEnd = 0;

	return pRp->screenData;
}
