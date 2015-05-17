#ifndef RT_INTERSECTION_CL
#define RT_INTERSECTION_CL

#include "CL/rt_types.cl"
#include "CL/rt_funcs_math.cl"

inline int rt_ray_sphere_intersection( rt_ray *pRay, rt_sphere *pSphere, float *t )
{
	float b, c, d;
	float x1, x2; 
	rt_vector3 tmp;
	
	// solving quadratic equation
	// (t^2)*(d^2) - 2*t*d*(s-c) + (p-c)^2 - r^2 = 0
	// a == d^2, b == -2*d*(s-c), c == (p-c)^2 - r^2
	// where d - ray direction, c - sphere center, 
	// r - sphere radius, s - ray origin
	// d is always normalized, so a == 1

	// compute b
	tmp = pRay->src - pSphere->pos;
	b = dot( pRay->dest, tmp );

	b *= 2.0f; 

	// compute c
	tmp = pRay->src - pSphere->pos;
	c = dot ( tmp, tmp );

	c -= pSphere->rad * pSphere->rad;

	// compute discriminant
	d = b * b - 4 * c;

	// only complex solutions, so no intersection
	if ( d < 0.0f )
		return 0;

	d = sqrt(d);

	x1 = ( -b - d ) / 2.0f;
	x2 = ( -b + d ) / 2.0f;	

	// is sphere behind ray origin?
	if ( x2 < 0.0f )
		return 0;

	// find intersection point and normal for nearest solution
	if ( x1 < 0.0f )
	{
		// if ray intersects sphere from inside, reverse normal
		*t = x2;

		return -1;
	}
	else
	{
		*t = x1;

		return 1;
	}

	return 1;
}

inline int rt_ray_plane_intersection( rt_ray *pRay, rt_plane *pPlane, float *t )
{
	float a, b;
	rt_vector3 tmp;	

	// t = (n*p - n*s) / d*n = a/b
	// n - plane normal, s - ray origin, d - ray direction

	//compute b
	b = dot( pPlane->norm, pRay->dest );

	//compute a
	tmp = pPlane->pos - pRay->src;
	a = dot( pPlane->norm, tmp );

	*t = (a / b);

	//is plane behind ray origin?
	if ( *t < 0 )
		return 0;
	
	return 1;
}

inline void rt_aparallel_ray_plane_intersection( rt_ray *pRay, RT_AXIS axis, 
	float pos, float *t )
{
	switch ( axis )
	{
	case RT_AXIS_X:
		*t = (pos - pRay->src.x) * pRay->invDest.x;
		break;

	case RT_AXIS_Y:
		*t = (pos - pRay->src.y) * pRay->invDest.y;
		break;

	case RT_AXIS_Z:
		*t = (pos - pRay->src.z) * pRay->invDest.z;
		break;
	}
}

//baricentic test
inline int rt_ray_triangle_intersection( rt_cl_render_pipe_data *pRp, 
	rt_ray *pRay, rt_triangle *pTri, float *t, float *u, float *v )
{
	rt_vector3 e1, e2, p, q, dist;
	rt_vector3 v0, v1, v2;
	float det;

	v0 = pRp->vertexBuf[pTri->v0].pos; 
	v1 = pRp->vertexBuf[pTri->v1].pos; 
	v2 = pRp->vertexBuf[pTri->v2].pos; 

	e1 = v1 - v0;
	e2 = v2 - v0;

	p = cross( pRay->dest, e2 );
	det = dot( e1, p );
		
	//if ( (det > -EPSILON) && (det < EPSILON) )
	if ( isequal(det, 0.0f) )
		return 0;
	
	det = 1.0f / det;

	*t = 0.0f;

	dist = pRay->src - v0;
	
	*u = dot( p, dist ) * det;
	if ( (*u < 0.0f) || (*u > 1.0f) )
		return 0;


	q = cross( dist, e1 );
	
	*v = dot( q, pRay->dest ) * det;
	if ( (*v < 0.0f) || (*v + *u > 1.0f) )
		return 0;

	*t = dot( q, e2 ) * det;

	if ( *t > 0.0f )
		return 1;

	return 0;
}

int rt_box_ray_intersection( rt_box *pC, rt_ray *pR, float *tNear, float *tFar )
{
	// 2D example, almost same for 3D
	//                        y
	//                   |    |    /--- t1
	//                   |    |   /| 
	//  -----------------|----|--/-|-------------------
	//		     |	  | /| |
	//                   |    |/ | |
	//		     |	  | t4 |
	//  _________________|___/|____|___________________ x
	//		     |	/ |    | 
	//  -----------------|-/--|----|-------------------
	//                   |/|  |    |
	//	      t2 ----/ |  |    |
	//                   | t3 |    |

	// invDest is precomputed
	// if r.dest == ( x, y, z ), then 
	// then r.invDest == ( 1/x, 1/y, 1/z )

	// x slab intersection
	float t1 = ( pC->center.x - pC->extents.x - pR->src.x ) * pR->invDest.x;
	float t2 = ( pC->center.x + pC->extents.x - pR->src.x ) * pR->invDest.x;

	// y slab intersection
	float t3 = ( pC->center.y - pC->extents.y - pR->src.y ) * pR->invDest.y;
	float t4 = ( pC->center.y + pC->extents.y - pR->src.y ) * pR->invDest.y;
	
	// z slab intersection
	float t5 = ( pC->center.z - pC->extents.z - pR->src.z ) * pR->invDest.z;
	float t6 = ( pC->center.z + pC->extents.z - pR->src.z ) * pR->invDest.z;

	// test
	*tNear = maxF( maxF(minF(t1, t2), minF(t3, t4)), minF(t5, t6) );
	*tFar = minF( minF(maxF(t1, t2), maxF(t3, t4)), maxF(t5, t6) );

	if ( *tFar < 0 )
		return 0;

	if ( *tNear > *tFar )
		return 0;

	return 1;
}

/*
int rt_cube_sphere_is_intersects( rt_cube *pC, rt_sphere *pS )
{
	float rSqr = (pS->rad) * (pS->rad);

	//x test
	if ( pS->pos.x < (pC->center.x - pC->extents) )
		rSqr -= pow( pS->pos.x - (pC->center.x - pC->extents), 2.0f );
	else
		if ( pS->pos.x > (pC->center.x + pC->extents) )
			rSqr -= pow( pS->pos.x - (pC->center.x + pC->extents),
			2.0f );

	//y test
	if ( pS->pos.y < (pC->center.y - pC->extents) )
		rSqr -= pow( pS->pos.y - (pC->center.y - pC->extents), 2.0f );
	else
		if ( pS->pos.y > (pC->center.y + pC->extents) )
			rSqr -= pow( pS->pos.y - (pC->center.y + pC->extents),
			2.0f );

	//z test
	if ( pS->pos.z < (pC->center.z - pC->extents) )
		rSqr -= pow( pS->pos.z - (pC->center.z - pC->extents), 2.0f );
	else
		if ( pS->pos.z > (pC->center.z + pC->extents) )
			rSqr -= pow( pS->pos.z - (pC->center.z + pC->extents),
			2.0f );


	return rSqr > 0.0f;
}

inline int vecprojintersects( rt_vector3 *cp, rt_vector3 *tp, rt_vector3 *v )
{
	float minTp, maxTp,
		minCp, maxCp, tmp;
	int i;

	//triangle verticles min and max dot-product
	maxTp = minTp = rt_vector3_dot( tp, v );
		
	tmp = rt_vector3_dot( tp + 1, v );
	maxTp = (tmp > maxTp) ? tmp : maxTp;
	minTp = (tmp < minTp) ? tmp : minTp;
	
	tmp = rt_vector3_dot( tp + 2, v );
	maxTp = (tmp > maxTp) ? tmp : maxTp;
	minTp = (tmp < minTp) ? tmp : minTp;
	
	//cube verticles min and max dot-product
	maxCp = minCp = rt_vector3_dot( cp, v );
	for ( i = 1; i < 8; ++i )
	{
		tmp = rt_vector3_dot( cp + i, v );
		
		maxCp = (tmp > maxCp) ? tmp : maxCp;
		minCp = (tmp < minCp) ? tmp : minCp;
	}

	//overlap test
	if ( minCp > maxTp )
		return 0;
	if ( minTp > maxCp )
		return 0;

	return 1;
}

int rt_cube_triangle_is_intersects( rt_cl_render_pipe_data *pRp, rt_cube *pC, rt_triangle *pT )
{
	rt_vector3 cp[8];
	rt_vector3 vp[3];
	rt_vector3 axs[3];
	rt_vector3 tedge[3];
	rt_vector3 tnorm;
	rt_vector3 crossProd;

	//translate everything, so that cube center will be (0, 0, 0)
	cp[0].x = -pC->extents; cp[0].y = -pC->extents; cp[0].z = -pC->extents;
	cp[1].x = -pC->extents; cp[1].y = -pC->extents; cp[1].z = +pC->extents;
	cp[2].x = -pC->extents; cp[2].y = +pC->extents; cp[2].z = -pC->extents;
	cp[3].x = -pC->extents; cp[3].y = +pC->extents; cp[3].z = +pC->extents;
	cp[4].x = +pC->extents; cp[4].y = -pC->extents; cp[4].z = -pC->extents;
	cp[5].x = +pC->extents; cp[5].y = -pC->extents; cp[5].z = +pC->extents;
	cp[6].x = +pC->extents; cp[6].y = +pC->extents; cp[6].z = -pC->extents;
	cp[7].x = +pC->extents; cp[7].y = +pC->extents; cp[7].z = +pC->extents;

	rt_vector3_sub( &(pC->center), &(pRp->vertexBuf[pT->pV0].pos), vp );
	rt_vector3_sub( &(pC->center), &(pRp->vertexBuf[pT->pV1].pos), vp+1 );
	rt_vector3_sub( &(pC->center), &(pRp->vertexBuf[pT->pV2].pos), vp+2 );

	axs[0].x = 1; axs[0].y = 0; axs[0].z = 0;
	axs[1].x = 0; axs[1].y = 1; axs[1].z = 0;
	axs[2].x = 0; axs[2].y = 0; axs[2].z = 1;

	//axes test
	if ( !vecprojintersects(cp, vp, axs) )
		return 0;
	if ( !vecprojintersects(cp, vp, axs+1) )
		return 0;
	if ( !vecprojintersects(cp, vp, axs+2) )
		return 0;
	
	//find 2 triangle edges and their cross-product (normal of triangle)
	rt_vector3_sub( vp + 0, vp + 1, tedge + 0 );
	rt_vector3_sub( vp + 1, vp + 2, tedge + 1 );
	rt_vector3_cross( tedge, tedge + 1, &tnorm );

	//triangle normal test
	if ( !vecprojintersects(cp, vp, &tnorm) )
		return 0;

	//third edge
	rt_vector3_sub( vp + 2, vp + 0, tedge + 2 );


	//cross products of all edges and axes test
	rt_vector3_cross( axs + 0, tedge + 0, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 0, tedge + 1, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 0, tedge + 2, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 1, tedge + 0, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 1, tedge + 1, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 1, tedge + 2, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 2, tedge + 0, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 2, tedge + 1, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	rt_vector3_cross( axs + 2, tedge + 2, &crossProd );
	if ( !vecprojintersects(cp, vp, &crossProd) )
		return 0;

	return 1;
}
*/
#endif
