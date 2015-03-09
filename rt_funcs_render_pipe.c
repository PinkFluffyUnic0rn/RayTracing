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

void rt_render_pipe_create( rt_render_pipe *pRp, int w, int h,
	void *pSD )
{
	if ( !pRp )
		exit( -1 );

	pRp->w = w;
	pRp->h = h;

	rt_init_opencl( pRp );
	rt_init_buffers( pRp );

	pRp->cam = malloc( sizeof(rt_camera) );
	pRp->screenData = (rt_argb *) malloc( sizeof(rt_argb) * h * w );

}

void rt_init_buffers( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	pRp->mempdecs = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_cl_prim_desc) * PRIMS_IN_BLOCK, NULL, NULL );
	pRp->memp = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		PRIMS_BLOCK_SIZE, NULL, NULL );
	pRp->primsEnd = 0;
	pRp->primsCount = 0;

	pRp->memldecs = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_cl_light_desc) * LIGHTS_IN_BLOCK, NULL, NULL );
	pRp->meml = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		PRIMS_BLOCK_SIZE, NULL, NULL );
	pRp->lightsEnd = 0;
	pRp->lightsCount = 0;

	pRp->memt = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_triangle)*TRIANGLES_IN_BLOCK, NULL, NULL );
	pRp->trianglesCount = 0;	

	pRp->memv = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_verticle)*VERTEX_IN_BLOCK, NULL, NULL );
	pRp->vertexCount = 0;	

	pRp->memm = clCreateBuffer( pRp->oclContent.context, CL_MEM_READ_ONLY, 
		sizeof(rt_material)*MATERIALS_IN_BLOCK, NULL, NULL );
}

void rt_render_pipe_set_camera( rt_render_pipe *pRp, 
	rt_camera *pFr )
{
	if ( (pRp == NULL) || (pFr == NULL) )
		exit( -1 );

	memcpy( pRp->cam, pFr, sizeof(rt_camera) );
}

void rt_render_pipe_add_primitive( rt_render_pipe *pRp, void *pPrim,
	RT_PRIMITIVE_TYPE pt )
{
	rt_cl_prim_desc *pClPDecs;
	void *memPtr;

	if ( (pRp == NULL) || (pPrim == NULL) )
		exit( -1 );

	if ( pRp->primsCount == PRIMS_IN_BLOCK )
		exit( -2 );

	pClPDecs = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->mempdecs, 
		CL_TRUE, CL_MAP_WRITE, 
		sizeof(rt_cl_prim_desc) * (pRp->primsCount), 
		sizeof(rt_cl_prim_desc), 0, NULL, NULL, NULL );

	pClPDecs->pt = pt;
	pClPDecs->offset = pRp->primsEnd;

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->mempdecs, pClPDecs, 0, 
		NULL, NULL );

	switch( pt )
	{
	case RT_PT_SPHERE:
		if ( (pRp->primsEnd + sizeof(rt_sphere)) >= PRIMS_BLOCK_SIZE )
			exit( -2 );
		
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
	rt_verticle *pV, rt_triangle *pTr, int vCount, int tCount )
{
	if ( (pRp == NULL) || (pV == NULL) || (pTr == NULL) )
		exit( -1 );
	
	if ( (pRp->trianglesCount + tCount) >= TRIANGLES_IN_BLOCK )
		exit( -2 );

	if ( (pRp->vertexCount + vCount) >= VERTEX_IN_BLOCK )
		exit( -2 );	

	clEnqueueWriteBuffer( pRp->oclContent.commQue, pRp->memv, 
		CL_TRUE, sizeof(rt_verticle)*(pRp->vertexCount), 
		sizeof(rt_verticle) * vCount, pV, 0, NULL, NULL );

	clEnqueueWriteBuffer( pRp->oclContent.commQue, pRp->memt, 
		CL_TRUE, sizeof(rt_triangle) * pRp->trianglesCount, 
		sizeof(rt_triangle) * tCount, pTr, 0, NULL, NULL );
	
	pRp->trianglesCount += tCount;
	pRp->vertexCount += vCount;
}

void rt_render_pipe_add_material( rt_render_pipe *pRp, rt_material *pMat, 
	unsigned long int ind )
{
	if ( (pRp == NULL) || (pMat == NULL) )
		exit( -1 );

	if ( (ind < 0) || (ind >= MATERIALS_IN_BLOCK) )
		exit( -2 );

	clEnqueueWriteBuffer( pRp->oclContent.commQue, pRp->memm, 
		CL_TRUE, sizeof(rt_material) * ind, 
		sizeof(rt_material), pMat, 0, NULL, NULL );
}

void rt_render_pipe_add_light( rt_render_pipe *pRp, void *pLight,
	RT_LIGHT_TYPE lt )
{
	int err;
	rt_cl_light_desc *pClLDecs;
	void *memPtr;

	if ( (pRp == NULL) || (pLight == NULL) )
		exit( -1 );

	pClLDecs = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memldecs, 
		CL_TRUE, CL_MAP_WRITE, 
		sizeof(rt_cl_light_desc) * (pRp->lightsCount), 
		sizeof(rt_cl_light_desc), 0, NULL, NULL, NULL );

	pClLDecs->lt = lt;
	pClLDecs->offset = pRp->lightsEnd;

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memldecs, pClLDecs, 0, 
		NULL, NULL );

	switch( lt )
	{
	case RT_LT_POINT:	
		if ( (pRp->lightsEnd + sizeof(rt_point_light)) >= LIGHTS_BLOCK_SIZE )
			exit( -2 );

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

	*ppCam = pRp->cam;
}

void rt_init_opencl( rt_render_pipe *pRp )
{
	int i;
	cl_int ret;
		
	//printf( "***: %s\n", rt_cl_raytrace_kernel_path );
	size_t progsrcsz;
	FILE *fp = fopen( rt_cl_raytrace_kernel_path, "r" );
	char *progsrc = (char *) malloc(0x10000000);
	

	if ( pRp == NULL )
		exit( -1 );

	rt_opencl_content *ocl = &(pRp->oclContent);

	ret = clGetPlatformIDs( 5, ocl->plID, &(ocl->plCount) );	
	for ( i = 0; i < ocl->plCount; ++i )
		clGetDeviceIDs( ocl->plID[i], CL_DEVICE_TYPE_ALL, 5, 
			(ocl->devID)[i], (ocl->devCount)+i );

	{
		char tmp[1024];

		clGetPlatformInfo( ocl->plID[0], CL_PLATFORM_NAME, 
			1024, tmp, NULL );
		printf( "choosen platform: %s\n", tmp );
		
		clGetDeviceInfo( ocl->devID[0][0], CL_DEVICE_NAME,
			1024, tmp, NULL );
		
		printf( "choosen device: %s\n", tmp );

		clGetDeviceInfo( ocl->devID[0][0], CL_DEVICE_MAX_COMPUTE_UNITS,
			sizeof(cl_uint), &(ocl->computeUnitsCount), NULL );

		printf( "compute units count: %u\n", ocl->computeUnitsCount );
	}
	
	ocl->context = clCreateContext( NULL, 1, (ocl->devID)[0], NULL, 
		NULL, &ret );
	ocl->commQue = clCreateCommandQueue( ocl->context, 
		(ocl->devID)[0][0], 0, &ret );
		
	if ( fp == NULL )
	{
		printf( "open file error\n" );
		exit( -3 );
	}
		
	progsrcsz = fread( progsrc, sizeof(char), 0x10000000, fp );
	fclose( fp );

	ocl->prog = clCreateProgramWithSource( ocl->context, 1, 
		(const char **)(&progsrc), 
		(const size_t *)(&progsrcsz), &ret );

	if ( ret != CL_SUCCESS )
	{
		printf( "clCreateProgramWithSource fail\n" );
		exit( -3 );
	}

	char *clArgs = malloc( sizeof(char) 
		* ( strlen(rt_cl_include_path) + 4) );

	strcpy( clArgs, "-I " );
	strcat( clArgs, rt_cl_include_path );

	ret = clBuildProgram( ocl->prog, 1, (ocl->devID)[0],
		clArgs, NULL, NULL );

	if ( ret != CL_SUCCESS )
	{
	    	size_t log_size;
   		char *log;
    			
		clGetProgramBuildInfo( ocl->prog, (ocl->devID)[0][0],
			CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size );

   		log = (char *) malloc(log_size);

    		clGetProgramBuildInfo( ocl->prog, (ocl->devID)[0][0],
			CL_PROGRAM_BUILD_LOG, log_size, log, NULL );

		printf( "clBuildProgram fail:\n" );
    		printf("%s\n", log);
			
		exit( -3 );
	}
	
	ocl->raytrace = clCreateKernel( ocl->prog, "raytrace", &ret );

	{
		size_t maxSz;

		clGetDeviceInfo( ocl->devID[0][0], 
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
}

void rt_opencl_render( rt_render_pipe *pRp )
{
	rt_cl_raytrace_args *rtArgs;
	void *memPtr;
	rt_opencl_content *pOcl = &(pRp->oclContent);
	cl_mem mema;
	cl_mem memc;
	cl_mem memout;

	if ( pRp == NULL )
		exit( -1 );

	// first arg
	mema = clCreateBuffer( pOcl->context, CL_MEM_READ_ONLY, 
		sizeof(rt_cl_raytrace_args), NULL, NULL );

	rtArgs = clEnqueueMapBuffer( pOcl->commQue, mema, CL_TRUE, CL_MAP_WRITE, 0, 
		sizeof(rt_cl_raytrace_args), 0, NULL, NULL, NULL );
	
	rtArgs->primsCount = pRp->primsCount;
	rtArgs->trianglesCount = pRp->trianglesCount;
	rtArgs->vertexCount = pRp->vertexCount;
	rtArgs->lightsCount = pRp->lightsCount;
	rtArgs->fillCol = pRp->fillCol;
	rtArgs->w = pRp->w;
	rtArgs->h = pRp->h;
	rtArgs->xdelta = 1;
	rtArgs->ydelta = 1;

	rtArgs->boundingBox = pRp->boundingBox;

	clEnqueueUnmapMemObject( pOcl->commQue, mema, rtArgs, 0, 
		NULL, NULL );

	// sixth arg
	memc = clCreateBuffer( pOcl->context, CL_MEM_READ_ONLY, 
		sizeof(rt_camera), NULL, NULL );

	memPtr = clEnqueueMapBuffer( pOcl->commQue, memc, CL_TRUE, CL_MAP_WRITE, 0, 
		sizeof(rt_camera), 0, NULL, NULL, NULL );

	memcpy( memPtr, pRp->cam, sizeof(rt_camera) );

	clEnqueueUnmapMemObject( pOcl->commQue, memc, memPtr, 0, 
		NULL, NULL );

	// output arg
	memout = clCreateBuffer( pOcl->context, CL_MEM_WRITE_ONLY, 
		sizeof(rt_argb)*(pRp->w)*(pRp->h), NULL, NULL );
	
	// enqueue threads
	{
		size_t gwSz[2] = { pRp->w, pRp->h };

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
			sizeof(cl_mem), (void *) &memc );
		clSetKernelArg( pOcl->raytrace, 6,
			sizeof(cl_mem), (void *) &(pRp->memm) );
		clSetKernelArg( pOcl->raytrace, 7,
			sizeof(cl_mem), (void *) &(pRp->memt) );
		clSetKernelArg( pOcl->raytrace, 8,
			sizeof(cl_mem), (void *) &(pRp->memv) );
		clSetKernelArg( pOcl->raytrace, 9,
			sizeof(cl_mem), (void *) &(pRp->memn) );
		clSetKernelArg( pOcl->raytrace, 10,
			sizeof(cl_mem), (void *) &(pRp->memi) );
		clSetKernelArg( pOcl->raytrace, 11,
			sizeof(cl_mem), (void *) &memout );

		clEnqueueNDRangeKernel( pOcl->commQue, pOcl->raytrace, 2, 
			NULL, gwSz, pOcl->workGroupSz, 0, NULL, NULL );	
	}
	
	clEnqueueReadBuffer( pOcl->commQue, memout, CL_TRUE, 0, 
		(pRp->w) * (pRp->h) * sizeof(rt_argb), pRp->screenData, 
		0, NULL, NULL );

	clReleaseMemObject( mema );	
	clReleaseMemObject( memc );	
	clReleaseMemObject( memout );
}

void rt_cleanup( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	free( pRp->cam );
	free( pRp->screenData );

	clReleaseMemObject( pRp->memp );	
	clReleaseMemObject( pRp->mempdecs );
	clReleaseMemObject( pRp->meml );	
	clReleaseMemObject( pRp->memldecs );
	clReleaseMemObject( pRp->memt );
	clReleaseMemObject( pRp->memv );
	clReleaseMemObject( pRp->memm );
}

void rt_kdtree_compute_sah( rt_ulong *memTp, rt_triangle *memTr, 
	rt_verticle *memVer, rt_ulong primsCount, RT_AXIS axis, 
	rt_box *pBox, rt_ulong *primsCountL, rt_ulong *primsCountR, 
	float *sep )
{
	float minSAH;
	float min, max;
	float delta;
	int b = 1;
	
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
	
	for ( ; min < max; min += delta )
	{
		float sah;
		int i;
		rt_ulong tmpPrimsCountL = 0;
		rt_ulong tmpPrimsCountR = 0;
	
		for ( i = 0; i < primsCount; ++i )
		{
			float maxV, minV;
			rt_triangle *tmpTr = memTr + memTp[i];

			switch ( axis )
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


			if ( minV < min )
				++tmpPrimsCountL;
			if ( maxV > min )
				++tmpPrimsCountR;
			
		}
		
		switch ( axis )
		{
		case RT_AXIS_X:
			sah = (pBox->extents.y + pBox->extents.z
				+ min - pBox->center.x + pBox->extents.x)
				* tmpPrimsCountL
				+ 
				(pBox->extents.y + pBox->extents.z
				+ pBox->center.x + pBox->extents.x - min) 
				* tmpPrimsCountR;
			break;

		case RT_AXIS_Y:
			sah = (pBox->extents.z + pBox->extents.x
				+ min - pBox->center.y + pBox->extents.y)
				* tmpPrimsCountL
				+ 
				(pBox->extents.z + pBox->extents.x
				+ pBox->center.y + pBox->extents.y - min) 
				* tmpPrimsCountR;
			break;
		case RT_AXIS_Z:
			sah = (pBox->extents.y + pBox->extents.x
				+ min - pBox->center.z + pBox->extents.z)
				* tmpPrimsCountL
				+ 
				(pBox->extents.y + pBox->extents.x
				+ pBox->center.z + pBox->extents.z - min) 
				* tmpPrimsCountR;
			break;
		}

		if ( sah < minSAH || b )
		{
			minSAH = sah;
			*sep = min;
			*primsCountL = tmpPrimsCountL;
			*primsCountR = tmpPrimsCountR;
		}
		
		b = 0;
	}
}

//with destruction
rt_kdtree_count_info rt_kdtree_pack_to_buffer( rt_cl_kdtree_node *pNodeBuf, 
	rt_ulong *pPrimsIdxBuf, rt_kdtree_node *pNode, 
	rt_ulong writePos, rt_ulong primsWritePos )
{
	rt_kdtree_count_info leftChilds, rightChilds;

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

rt_kdtree_count_info rt_kdtree_make_childs( rt_verticle *memVer, rt_triangle *memTr, 
	rt_kdtree_node *pNode, rt_box *bbox, 
	int depth )
{
	float range = bbox->extents.x;
	rt_ulong i, j, k;
	rt_box tmpBoxL, tmpBoxR;
	
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

	pNode->leftNode = malloc( sizeof(rt_kdtree_node) );
	pNode->rightNode = malloc( sizeof(rt_kdtree_node) );
	
	rt_kdtree_compute_sah( pNode->prims, memTr, memVer, pNode->primsCount, 
		pNode->axis, bbox, &(pNode->leftNode->primsCount), 
		&(pNode->rightNode->primsCount), &(pNode->sep) );

	pNode->leftNode->prims = malloc( sizeof(rt_ulong) * pNode->leftNode->primsCount );
	pNode->rightNode->prims = malloc( sizeof(rt_ulong) * pNode->rightNode->primsCount );

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

void rt_kdtree_build( rt_render_pipe *pRp )
{
	rt_triangle *memTr;
	rt_verticle *memVer;
	rt_ulong i;
	rt_vector3 minP, maxP;
	rt_ulong primsInNodesCount;
	rt_ulong nodesCount;
	rt_kdtree_node *rootNode = malloc( sizeof(rt_kdtree_node) );

	memTr = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memt, 
		CL_TRUE, CL_MAP_WRITE, 0, sizeof(rt_triangle) 
		* (pRp->trianglesCount), 0, NULL, NULL, NULL );
	memVer = clEnqueueMapBuffer( pRp->oclContent.commQue, pRp->memv, 
		CL_TRUE, CL_MAP_WRITE, 0, sizeof(rt_verticle) 
		* (pRp->vertexCount), 0, NULL, NULL, NULL );

	for ( i = 0; i < pRp->trianglesCount; ++i )
	{
		minP.x = (minP.x > (memVer + memTr[i].pV0)->pos.x || i == 0) ? 
			(memVer + memTr[i].pV0)->pos.x : minP.x;
		minP.y = (minP.y > (memVer + memTr[i].pV1)->pos.y || i == 0) ? 
			(memVer + memTr[i].pV1)->pos.y : minP.y;		
		minP.z = (minP.z > (memVer + memTr[i].pV2)->pos.z || i == 0) ? 
			(memVer + memTr[i].pV2)->pos.z : minP.z;

		maxP.x = (maxP.x < (memVer + memTr[i].pV0)->pos.x || i == 0) ? 
			(memVer + memTr[i].pV0)->pos.x : maxP.x;
		maxP.y = (maxP.y < (memVer + memTr[i].pV1)->pos.y || i == 0) ? 
			(memVer + memTr[i].pV1)->pos.y : maxP.y;		
		maxP.z = (maxP.z < (memVer + memTr[i].pV2)->pos.z || i == 0) ? 
			(memVer + memTr[i].pV2)->pos.z : maxP.z;	
	}

	rt_vector3_add( &maxP, &minP, &(pRp->boundingBox.center) );
	rt_vector3_scalar_mult( &(pRp->boundingBox.center), 0.5f, 
		&(pRp->boundingBox.center) );

	rt_vector3_sub( &minP, &maxP, &(pRp->boundingBox.extents) );
	rt_vector3_scalar_mult( &(pRp->boundingBox.extents), 0.5f, 
		&(pRp->boundingBox.extents) );


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
			rootNode, &(pRp->boundingBox), 0 );

		primsInNodesCount = tmpCntI.primsCount;
		nodesCount = tmpCntI.nodesCount;
	}

	{
		rt_cl_kdtree_node *memNode;
		rt_ulong *memTrIdx;

		pRp->memi = clCreateBuffer( pRp->oclContent.context, 
			CL_MEM_READ_ONLY, sizeof(rt_ulong) * primsInNodesCount, 
			NULL, NULL );
		pRp->memn = clCreateBuffer( pRp->oclContent.context, 
			CL_MEM_READ_ONLY, 
			sizeof(rt_cl_kdtree_node) * nodesCount, NULL, NULL );

		memNode = clEnqueueMapBuffer( pRp->oclContent.commQue, 
			pRp->memn, CL_TRUE, CL_MAP_WRITE, 0, 
			sizeof(rt_cl_kdtree_node) * nodesCount, 
			0, NULL, NULL, NULL );
		memTrIdx = clEnqueueMapBuffer( pRp->oclContent.commQue, 
			pRp->memi, CL_TRUE, CL_MAP_WRITE, 0, 
			sizeof(rt_ulong) * primsInNodesCount, 
			0, NULL, NULL, NULL );

		rt_kdtree_pack_to_buffer( memNode, 
			memTrIdx, rootNode, 0, 0 );

		clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memn, 
			memNode, 0, NULL, NULL );
	 	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memi, 
			memTrIdx, 0, NULL, NULL );
	}

	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memv, memVer, 0, 
		NULL, NULL );
	clEnqueueUnmapMemObject( pRp->oclContent.commQue, pRp->memt, memTr, 0, 
		NULL, NULL );
}


rt_argb *rt_render_pipe_draw( rt_render_pipe *pRp )
{
	if ( pRp == NULL )
		exit( -1 );

	rt_kdtree_build( pRp );
	rt_opencl_render( pRp );

	clReleaseMemObject( pRp->memi );
	clReleaseMemObject( pRp->memn );

	pRp->trianglesCount = 0;
	pRp->vertexCount = 0;
	pRp->lightsCount = 0;
	pRp->primsCount = 0;
	pRp->primsEnd = 0;
	pRp->lightsEnd = 0;

	return pRp->screenData;
}
