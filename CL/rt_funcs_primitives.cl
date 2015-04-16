#ifndef RT_FUNCS_PRIMITIVES_CL
#define RT_FUNCS_PRIMITIVES_CL

#include "CL/rt_types.cl"
#include "CL/rt_funcs_math.cl"
#include "CL/rt_intersection.cl"


inline ulong rt_primitive_get_material( rt_cl_render_pipe_data *pRp, 
	ulong prN )
{
	ulong n;
	
	switch ( pRp->primsDecs[prN].pt )
	{
	case RT_PT_SPHERE:
		n = ((__global const rt_sphere *)(pRp->primsBuf
			+ pRp->primsDecs[prN].offset))->mat;
		break;
	case RT_PT_PLANE:
		n = ((__global const rt_plane *)(pRp->primsBuf
			+ pRp->primsDecs[prN].offset))->mat;
		break;

	default:
		break;
	}

	return n;
}

inline void rt_get_point_from_ray( rt_cl_render_pipe_data *pRp, rt_ray *pRay, 
	int b, ulong prN, float t, rt_vector3 *pPoint, rt_vector3 *pNormal )
{
	switch ( pRp->primsDecs[prN].pt )
	{
	case RT_PT_SPHERE:
		{
			rt_sphere pSphere 
				= *((__global const rt_sphere *)(pRp->primsBuf+
				pRp->primsDecs[prN].offset));

			*pPoint = pRay->dest * t;
			*pPoint += pRay->src;

			*pNormal = (*pPoint - pSphere.pos) / pSphere.rad;
			*pNormal *= (float) b;

			break;
		}

	case RT_PT_PLANE:	
		*pPoint = pRay->dest * t;
		*pPoint += pRay->src;
		*pNormal = ((rt_plane *)(pRp->primsDecs[prN].offset))->norm; 
		break;

	default:
		break;
	}
}

inline void rt_get_triangle_point( rt_cl_render_pipe_data *pRp, rt_ray *pRay, 
	int b, ulong prN, float t, float u, float v, rt_vector3 *pPoint, 
	rt_vector3 *pNormal )
{
	*pPoint = pRay->dest * t + pRay->src;
		
	*pNormal = u*pRp->vertexBuf[pRp->trianglesBuf[prN].v1].norm
		+ v*pRp->vertexBuf[pRp->trianglesBuf[prN].v2].norm 
		+ ( 1.0f - u - v )
		* pRp->vertexBuf[pRp->trianglesBuf[prN].v0].norm;
}

inline void rt_get_nearest_in_last( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	ulong nodeIdx, float *minT, float *minU, float *minV,
	ulong *prN, int *nearestB )
{
	ulong i;
	ulong intrsC = 0;
	ulong sz = pRp->kdtreeNodesBuf[nodeIdx].primsCount;
	
	for ( i = 0; i < sz; ++i )
	{
		int b;
		rt_triangle tmpT;
		float t, u, v;
		ulong primsOffset = pRp->kdtreePrimsIdxBuf[
			pRp->kdtreeNodesBuf[nodeIdx].prims + i];
		
		tmpT = (pRp->trianglesBuf)[primsOffset];
		b = rt_ray_triangle_intersection( pRp, pR, &tmpT, &t, &u, &v );
	

		if ( b )
		{
			++intrsC;

			if ( (intrsC == 1) || (t < *minT) )
			{
				*minT = t;
				*minU = u;
				*minV = v;
				
				*prN = primsOffset;
				*nearestB = b;
			}
		}
	}

}

/*
inline void rt_get_nearest_triangle( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	float *minT, float *minU, float *minV,
	ulong *prN, int *nearestB )
{
	ulong i;
	*nearestB = 0;
	ulong intrsC = 0;

	for ( i = 0; i < pRp->trianglesCount; ++i )
	{
		int b;
		rt_triangle tmpT;
		float t, u, v;
		
		tmpT = (pRp->trianglesBuf)[i];
		
		b = rt_ray_triangle_intersection( pRp, pR, &tmpT, &t, &u, &v );
	

		if ( b )
		{
			++intrsC;

			if ( (intrsC == 1) || (t < *minT) )
			{
				*minT = t;
				*minU = u;
				*minV = v;
				
				*prN = i;
				*nearestB = b; 
			}
		}
	}
}
*/

inline void rt_get_nearest_triangle( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	float *minT, float *minU, float *minV, ulong *prN, int *nearestB )
{
	float tNear, tFar, tSplit;
	ulong currentNode = 0;
	ulong nearNode, farNode;
	ulong stack[KDTREE_DEPTH];
	float stackFar[KDTREE_DEPTH];
	int stackPos = 0;


	if ( !rt_box_ray_intersection( &(pRp->boundingBox), pR, &tNear, &tFar ) )
		return;

	while ( 1 )
	{
		if ( pRp->kdtreeNodesBuf[currentNode].isLast )
		{
			rt_get_nearest_in_last( pRp, pR, currentNode, minT, minU, minV,
				prN, nearestB );
		
			if ( *nearestB && (*minT < tFar) )
				return;
		
			if ( stackPos )
			{
				--stackPos;
				currentNode = stack[stackPos];
				tNear = tFar;
				tFar = stackFar[stackPos];	
			}
			else
				return;
		}
		else
		{
			float sep = pRp->kdtreeNodesBuf[currentNode].sep;	
		

			switch ( pRp->kdtreeNodesBuf[currentNode].axis )
			{
			case RT_AXIS_X:
				tSplit = (sep - pR->src.x) * pR->invDest.x;
				break;

			case RT_AXIS_Y:
				tSplit = (sep - pR->src.y) * pR->invDest.y;
				break;

			case RT_AXIS_Z:
				tSplit = (sep - pR->src.z) * pR->invDest.z;
				break;
			}


			nearNode = pRp->kdtreeNodesBuf[currentNode].leftNode;
			farNode = pRp->kdtreeNodesBuf[currentNode].rightNode;


			switch ( pRp->kdtreeNodesBuf[currentNode].axis )
			{
			case RT_AXIS_X:
				if ( pR->dest.x < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
	
			case RT_AXIS_Y:
				if ( pR->dest.y < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
	
			case RT_AXIS_Z:
				if ( pR->dest.z < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
			}

			if ( tSplit >= tFar )
				currentNode = nearNode;
			else if ( tSplit <= tNear )
				currentNode = farNode;
			else
			{
				stack[stackPos] = farNode;
				stackFar[stackPos] = tFar;
				++stackPos;

				currentNode = nearNode;
				tFar = tSplit;
			}
			
		}
	}

}


inline void rt_add_alpha_in_last( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	ulong nodeIdx, float d, float tFar, float *alpha)
{
	ulong i;
	ulong sz = pRp->kdtreeNodesBuf[nodeIdx].primsCount;

	for ( i = 0; i < sz; ++i )
	{
		int b;
		rt_triangle tmpT;
		float t, u, v;
		ulong primsOffset = pRp->kdtreePrimsIdxBuf[
			pRp->kdtreeNodesBuf[nodeIdx].prims + i];
		
		tmpT = (pRp->trianglesBuf)[primsOffset];
		b = rt_ray_triangle_intersection( pRp, pR, &tmpT, &t, &u, &v );
	

		if ( b && (t < d) && (t < tFar) )
		{
			
			*alpha += pRp->materialBuf[pRp->trianglesBuf[primsOffset].mat].color.a;

		}
	}
}

inline void rt_get_alpha_triangles( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	float d, float *alpha )
{
	float tNear, tFar, tSplit;
	ulong currentNode = 0;
	ulong nearNode, farNode;
	ulong stack[KDTREE_DEPTH];
	float stackFar[KDTREE_DEPTH];
	int stackPos = 0;


	if ( !rt_box_ray_intersection( &(pRp->boundingBox), pR, &tNear, &tFar ) )
		return;

	*alpha = 0.0f;

	while ( 1 )
	{
		if ( pRp->kdtreeNodesBuf[currentNode].isLast )
		{
			rt_add_alpha_in_last( pRp, pR, currentNode, d, tFar, alpha );

// same primitive can be placed into several nodes,
// need to check this probability, when summing alphas
// shadows for triangles disabled in "rt_light_point" function
			
			if ( stackPos )
			{
				--stackPos;
				currentNode = stack[stackPos];
				tNear = tFar;
				tFar = stackFar[stackPos];
			}
			else
				return;
		}
		else
		{
			float sep = pRp->kdtreeNodesBuf[currentNode].sep;	
			
			switch ( pRp->kdtreeNodesBuf[currentNode].axis )
			{
			case RT_AXIS_X:
				tSplit = (sep - pR->src.x) * pR->invDest.x;
				break;

			case RT_AXIS_Y:
				tSplit = (sep - pR->src.y) * pR->invDest.y;
				break;

			case RT_AXIS_Z:
				tSplit = (sep - pR->src.z) * pR->invDest.z;
				break;
			}
	
			nearNode = pRp->kdtreeNodesBuf[currentNode].leftNode;
			farNode = pRp->kdtreeNodesBuf[currentNode].rightNode;

			switch ( pRp->kdtreeNodesBuf[currentNode].axis )
			{
			case RT_AXIS_X:
				if ( pR->dest.x < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
	
			case RT_AXIS_Y:
				if ( pR->dest.y < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
	
			case RT_AXIS_Z:
				if ( pR->dest.z < 0.0f )
				{
					ulong tmp = nearNode;
					nearNode = farNode;
					farNode = tmp;
				}
				break;
			}

			if ( tSplit >= tFar )
				currentNode = nearNode;
			else if ( tSplit <= tNear )
				currentNode = farNode;
			else
			{
				stack[stackPos] = farNode;
				stackFar[stackPos] = tFar;
				++stackPos;

				currentNode = nearNode;
				tFar = tSplit;
			}
		}
	}
}

inline void rt_get_nearest_prim( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	float *minT, ulong *prN, int *nearestB )
{
	ulong i;


	for ( i = 0; i < pRp->primsCount; ++i )
	{
		int b;
		float t;
		
		RT_PRIMITIVE_TYPE tmp = pRp->primsDecs[i].pt;

		if ( tmp == RT_PT_SPHERE )
		{
			rt_sphere tmpS 
				= *((__global const rt_sphere *)(pRp->primsBuf
				+ pRp->primsDecs[i].offset));
			b = rt_ray_sphere_intersection( pR, &tmpS, &t );	
		}


		if ( b )
		{
			if ( !(*nearestB) || (t < *minT) )
			{
				*minT = t;
				*prN = i;
				*nearestB = b; 
			}
		}

	}
}

inline void rt_get_alpha_prims( rt_cl_render_pipe_data *pRp, rt_ray *pR, 
	float d, float *alpha )
{
	ulong i;

	*alpha = 0.0f;

	for ( i = 0; i < pRp->primsCount; ++i )
	{
		int b;
		float t;
		
		RT_PRIMITIVE_TYPE tmp = pRp->primsDecs[i].pt;

		if ( tmp == RT_PT_SPHERE )
		{
			rt_sphere tmpS 
				= *((__global const rt_sphere *)(pRp->primsBuf
				+ pRp->primsDecs[i].offset));
			b = rt_ray_sphere_intersection( pR, &tmpS, &t );	
		}


		if ( b && t < d )
		{
			*alpha += pRp->materialBuf[((__global const rt_sphere *)(pRp->primsBuf
				+ pRp->primsDecs[i].offset))->mat].color.a;
		}

	}
}

inline void rt_light_point( rt_cl_render_pipe_data *pRp, rt_vector3 *pV,
	rt_vector3 *pN, rt_color *pCol, rt_material *pMat, 
	rt_vector3 *pViewerPos )
{
	ulong o;
	rt_color tmpCol;

	rt_color_create( pCol, 0.0f, 0.0f, 0.0f, 0.0f );

	for ( o = 0; o < pRp->lightsCount; ++o )
		if ( (pRp->lightsDecs)[o].lt == RT_LT_POINT )
		{
			rt_point_light tmpL;
			float d; 
 			rt_vector3 toLight;
			float ang;
			rt_color diffuse;
			rt_color specular;
			float shadowed = 0.0f;

			rt_color_create( &specular, 0.0f, 0.0f, 0.0f, 0.0f );
			rt_color_create( &diffuse, 0.0f, 0.0f, 0.0f, 0.0f );

			tmpL = *((__global const rt_point_light *)(pRp->lightsBuf
				+pRp->lightsDecs[o].offset));

			toLight = tmpL.pos - *pV;
			d = length( toLight );
			toLight /= d;

			if ( SHADOWS_ENABLED )
			{
				rt_ray shadowRay;
			
				float t;
				float u, v;
				int nearestB = 0;
				int prN;
				
				float a;
				
				shadowRay.dest = toLight;
				shadowRay.src = *pV + toLight * EPSILON;
				shadowRay.invDest = 1.0f / toLight;

				rt_get_alpha_prims( pRp, &shadowRay, d, &a );
				
				shadowed = rt_clamp_float(a, 0.0f, 1.0f);

//				rt_get_alpha_triangles( pRp, &shadowRay, d, &a ); 

//				shadowed += rt_clamp_float(a, 0.0f, 1.0f);
				shadowed *= pMat->color.a;
			}

			if ( DIFFUSE_ENABLED )
			{
				ang = max( 0.0f, dot( *pN, toLight ));

				diffuse = tmpL.col;

				rt_color_mult( &diffuse, &(pMat->diffuse), 
					&diffuse );

				rt_color_mult( &diffuse, &(pMat->color), 
					&diffuse );

				rt_color_scalar_mult( &diffuse, 
					ang * (tmpL.rad) / d, 
					&diffuse );

			}
			else
				rt_color_create( &diffuse, 0.0f, pMat->color.r,
					pMat->color.g, pMat->color.b );

			if ( SPECULAR_ENABLED )
			{
				float specFact;	
				rt_vector3 toEye;

				toLight *= -1.0f;
				toLight = rt_vector3_reflect( toLight, *pN );

				toEye = *pViewerPos - *pV;
			
				toEye = normalize( toEye );
								
				ang = dot( toEye, toLight );

				specFact = pow( max(ang, 0.0f),
					pMat->specular.a );

				specular = pMat->specular;
	
				rt_color_mult( &specular, &(tmpL.col), 
					&specular );

				rt_color_scalar_mult( &specular, 
					specFact, &specular );
			} 
		
			rt_color_add( &diffuse, &specular, &tmpCol );
			rt_color_scalar_mult( &tmpCol, 1.0f - shadowed, &tmpCol );
			rt_color_add( &tmpCol, pCol, pCol );
		}
}
/*
inline rt_color rt_raytrace( rt_cl_render_pipe_data *pRp, 
	rt_ray *pR, int depth )
{
	float trMinT, trMinU, trMinV;
	ulong trN;
	int trNearestB = 0;

	rt_color col = pRp->fillCol;

	rt_get_nearest_triangle( pRp, pR, &trMinT, &trMinU, &trMinV,
		&trN, &trNearestB ); 
	if ( trNearestB != 0 )
		col = pRp->materialBuf[pRp->trianglesBuf[trN].mat].color;

	return col;
}
*/

inline rt_color rt_raytrace( rt_cl_render_pipe_data *pRp, 
	rt_ray *pR, int depth )
{
	rt_ray rays[NODES_COUNT];
	rt_color col[NODES_COUNT];
	rt_vector3 p[NODES_COUNT];
	rt_vector3 n[NODES_COUNT];
	ulong pPrMat[NODES_COUNT];
	int bNearest[NODES_COUNT];
	rt_material tmpMat;
	ulong i;
	rays[0] = *pR;
	
	for ( i = 0; i < NODES_COUNT; ++i )
	{
		float prMinT, trMinT, trMinU, trMinV;
		ulong prN, trN;
		int prNearestB = 0, trNearestB = 0;
		float nRel, cosI, cosT;

		col[i] =  pRp->fillCol;
		
		if ( (i != 0) && (bNearest[(i-1) / 2] == 0) )
		{
			bNearest[i] = 0;
			continue;
		}
		
		rt_get_nearest_prim( pRp, rays + i, &prMinT, &prN, &prNearestB );
		rt_get_nearest_triangle( pRp, rays + i, &trMinT, &trMinU, &trMinV,
			&trN, &trNearestB ); 

		if ( (trNearestB == 0) && (prNearestB == 0) )
		{
			bNearest[i] = 0;
			continue;
		}


		if ( (trNearestB && !prNearestB)
			 || (trNearestB && prNearestB && trMinT < prMinT) )
		{
			rt_triangle tr = pRp->trianglesBuf[trN];

			pPrMat[i] = tr.mat;
			bNearest[i] = trNearestB;

			rt_get_triangle_point( pRp, rays + i, trNearestB, trN, 
				trMinT, trMinU, trMinV, p + i, n + i );
		}

		if ( (!trNearestB && prNearestB)
			 || (trNearestB && prNearestB && prMinT < trMinT) )
		{
			pPrMat[i] = rt_primitive_get_material( pRp, prN );
			bNearest[i] = prNearestB;

			rt_get_point_from_ray( pRp, rays + i, prNearestB, prN, 
				prMinT, p + i, n + i );
		}
		
		if ( i >= LIM_NEW_RAYS )
		{
			rt_material tmpMat = pRp->materialBuf[pPrMat[i]];
		
			rt_light_point( pRp, p + i, n + i, 
				col + i, &tmpMat, 
				&(rays[i].src) );
	
			continue;
		}

		//calculate reflected ray
		rays[2 * i + 1].dest = rt_vector3_reflect( rays[i].dest, n[i] );
		rays[2 * i + 1].src = p[i] + rays[2 * i + 1].dest * EPSILON;
		rays[2 * i + 1].invDest = 1.0f / rays[2 * i + 1].dest;

		//calculate refracted ray	
		nRel = ENV_OPT_DENSITY / pRp->materialBuf[pPrMat[i]].optDens;
		cosI = -dot( n[i], rays[i].dest );
		cosT = 1.0f - nRel * nRel * ( 1.0f - cosI * cosI );

		if ( cosT < 0.0f )
		{
			bNearest[2 * i + 2] = 0;
			continue;
		}

		rays[2 * i + 2].dest = normalize( rays[i].dest * nRel 
			+ n[i] * (nRel * cosI - sqrt( cosT )) );
		rays[2 * i + 2].src = p[i] + rays[2 * i + 2].dest * EPSILON;
		rays[2 * i + 2].invDest = 1.0f / rays[2 * i + 2].dest;
	}

	if ( !bNearest[0] )
		return pRp->fillCol;

	for ( i = NODES_COUNT-1; i > 0; --i )
	{
		ulong parentIdx = (i - 1) / 2;

		if (bNearest[parentIdx] == 0) 
			continue;

		if ( i % 2 )
		{
			if ( bNearest[parentIdx] == 1 )
			{
 				rt_material tmpMat = pRp->materialBuf[pPrMat[parentIdx]];
				rt_color primRef = tmpMat.reflect;

				col[parentIdx].r += col[i].r * primRef.r;
				col[parentIdx].g += col[i].g * primRef.g;
				col[parentIdx].b += col[i].b * primRef.b;

			}
		}
		else
		{
			rt_color primCol = pRp->materialBuf[pPrMat[parentIdx]].color;
			float alpha = pRp->materialBuf[pPrMat[parentIdx]].color.a;
			float dist;
			float refr = ( 1.0f - alpha );
 			rt_material tmpMat = pRp->materialBuf[pPrMat[parentIdx]];
		
			rt_light_point( pRp, p + parentIdx, n + parentIdx, 
				col + parentIdx, &tmpMat, &(rays[parentIdx].src) );

			if ( bNearest[i] )
			{
				dist = length( p[parentIdx] - p[i] );
				refr *= exp( pRp->materialBuf[
					pPrMat[parentIdx]].lightFalloff
					* -dist );
			}

			col[parentIdx].r += col[i].r * refr;
			col[parentIdx].g += col[i].g * refr;
			col[parentIdx].b += col[i].b * refr;
		}
				
		rt_color_clamp( col + parentIdx, col + parentIdx );
	}
	
	return col[0];
}

#endif
