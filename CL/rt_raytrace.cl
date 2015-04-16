#include "CL/rt_types.cl"
#include "CL/rt_funcs_math.cl"
#include "CL/rt_intersection.cl"
#include "CL/rt_funcs_primitives.cl"

__kernel void raytrace( __constant rt_cl_raytrace_args *a,
			__global const void *prims,
			__global const rt_cl_prim_desc *pd, 
			__global const void *lights,
			__global const rt_cl_light_desc *ld, 
			__constant rt_camera *cams,
			__global const rt_material *materials,
			__global const rt_triangle *triangles,
			__global const rt_vertex *verticies,
			__global const rt_cl_kdtree_node *kdtreeNodes,
			__global const ulong *kdtreePrimsIdx,
			__global rt_argb *o )
{
	rt_camera pCurCam = *(cams);
	rt_cl_render_pipe_data pRpData;
	rt_ray projRay;
	rt_color col;

	int y = get_global_id(1) * (a->ydelta);
	int x = get_global_id(0) * (a->xdelta);
	int i, j;
	float xTr = 2.0f/(float)(a->w-1);
	float yTr = 2.0f/(float)(a->h-1);
	
	pRpData.boundingBox = a->boundingBox;
	pRpData.fillCol = a->fillCol;
	
	pRpData.primsBuf = prims;
	pRpData.primsDecs = pd;
	pRpData.lightsBuf = lights;
	pRpData.lightsDecs = ld;
	pRpData.materialBuf = materials;
	pRpData.trianglesBuf = triangles;
	pRpData.vertexBuf = verticies;
	pRpData.kdtreeNodesBuf = kdtreeNodes;
	pRpData.kdtreePrimsIdxBuf = kdtreePrimsIdx;

	pRpData.primsCount = a->primsCount;
	pRpData.lightsCount = a->lightsCount;
	pRpData.trianglesCount = a->trianglesCount;	
	pRpData.vertexCount = a->vertexCount;	
		
	pRpData.w = a->w;
	pRpData.h = a->h;
	
	for ( i = 0; i < a->ydelta; ++i )
		for ( j = 0; j < a->xdelta; ++j )
		{
			projRay.src.x = projRay.src.y = projRay.src.z = 0.0f;
			projRay.dest.z = 1.0f;	
			projRay.dest.x = xTr*(float)(x+j) - 1.0f;
			projRay.dest.y = yTr*(float)(y+i) - 1.0f;

			rt_vector3_matrix4_mult( projRay.src,
				&(pCurCam.world), 
				&(projRay.src) );

			rt_vector3_matrix4_mult_dir( projRay.dest,
				&(pCurCam.viewToPersp), 
				&(projRay.dest) );
	
			rt_vector3_matrix4_mult_dir( projRay.dest,
				&(pCurCam.world), 
				&(projRay.dest) );	

			projRay.dest = normalize( projRay.dest );

			projRay.invDest = 1.0f / projRay.dest;

			col = rt_raytrace( &pRpData, &projRay, 0 );

			o[(y + i)*(a->w) + (x + j)].a = 255.0f*col.a;
			o[(y + i)*(a->w) + (x + j)].r = 255.0f*col.r;
			o[(y + i)*(a->w) + (x + j)].g = 255.0f*col.g;
			o[(y + i)*(a->w) + (x + j)].b = 255.0f*col.b;
		}

}

