#include "rt_funcs_primitives.h"

void rt_sphere_create( rt_sphere *pS, rt_vector3 *pP, 
	rt_float r, unsigned long int mat )
{
	if ( (pS == NULL) || (pP == NULL) )
		exit ( -1 );

	memcpy( &(pS->pos), pP, sizeof(rt_vector3) );
	pS->mat = mat;
	pS->rad = r;
}

void rt_plane_create( rt_plane *pPl, rt_vector3 *pP, 
	rt_vector3 *pN, unsigned long int mat )
{
	if ( (pPl == NULL) || (pP == NULL) || (pN == NULL) )
		exit ( -1 );

	memcpy( &(pPl->pos), pP, sizeof(rt_vector3) );
	memcpy( &(pPl->norm), pN, sizeof(rt_vector3) );
	pPl->mat = mat;
}

void rt_material_create( rt_material *pM, rt_color *pC, rt_color *pA,
	rt_color *pD, rt_color *pS, rt_color *pR, 
	rt_float rFoff, rt_float oD )
{
	if ( (pM == NULL) || (pC == NULL) || (pA == NULL) 
		|| (pD == NULL )|| (pS == NULL) || (pR == NULL) )
		exit( -1 );

	memcpy( &(pM->color), pC, sizeof(rt_color) );
	memcpy( &(pM->ambient), pA, sizeof(rt_color) );
	memcpy( &(pM->diffuse), pD, sizeof(rt_color) );
	memcpy( &(pM->specular), pS, sizeof(rt_color) );
	memcpy( &(pM->reflect), pR, sizeof(rt_color) );
	pM->lightFalloff = rFoff;
	pM->optDens = oD;
}

void rt_point_light_create( rt_point_light *pS, 
	rt_vector3 *pP, rt_float r, rt_color *pCol )
{
	if ( (pS == NULL) || (pP == NULL) )
		exit ( -1 );

	memcpy( &(pS->pos), pP, sizeof(rt_vector3) );
	memcpy( &(pS->col), pCol, sizeof(rt_color) );
	pS->rad = r;
}

void rt_camera_create( rt_camera *pC, rt_float asp, rt_float hV )
{
	if ( pC == NULL )
		exit( -1 );
	
	pC->world._11 = 1; pC->world._12 = 0;   
		pC->world._13 = 0; pC->world._14 = 0;
	pC->world._21 = 0; pC->world._22 = 1;   
		pC->world._23 = 0; pC->world._24 = 0;
	pC->world._31 = 0; pC->world._32 = 0;   
		pC->world._33 = 1; pC->world._34 = 0;
	pC->world._41 = 0; pC->world._42 = 0;   
		pC->world._43 = 0; pC->world._44 = 1;

	rt_matrix4_create_projection( &(pC->viewToPersp), 
		asp, hV );
}

void rt_camera_set_view_matrix( rt_camera *pC, rt_matrix4 *pM )
{
	if ( (pC == NULL) || (pM == NULL) )
		exit ( -1 );

	pC->world = *pM;
}

void rt_camera_set_perspective_matrix( rt_camera *pC, rt_matrix4 *pM )
{
	if ( (pC == NULL) || (pM == NULL) )
		exit ( -1 );

	pC->viewToPersp = *pM;
}
