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

void rt_camera_create( rt_camera *pC, int w, int h, rt_float hV, 
	rt_float fZ )
{
	if ( pC == NULL )
		exit( -1 );

	pC->farZ = fZ;
	pC->hView = hV;
	pC->wView = 

	pC->aspect = (rt_float)w / (rt_float)h;
	
	pC->dcToSsc._11 = 2.0f/(rt_float)(w-1); pC->dcToSsc._12 = 0;
		pC->dcToSsc._13 = 0;
	pC->dcToSsc._21 = 0; pC->dcToSsc._22 = 2.0f/(rt_float)(h-1); 
		pC->dcToSsc._23 = 0;
	pC->dcToSsc._31 = -1; pC->dcToSsc._32 = -1;          
		pC->dcToSsc._33 = 1;

	pC->world._11 = 1; pC->world._12 = 0;   
		pC->world._13 = 0; pC->world._14 = 0;
	pC->world._21 = 0; pC->world._22 = 1;   
		pC->world._23 = 0; pC->world._24 = 0;
	pC->world._31 = 0; pC->world._32 = 0;   
		pC->world._33 = 1; pC->world._34 = 0;
	pC->world._41 = 0; pC->world._42 = 0;   
		pC->world._43 = 0; pC->world._44 = 1;


	pC->viewToPersp._11 = 2.0f*tan(hV/2.0f)*(pC->aspect); 
		pC->viewToPersp._12 = 0.0f; pC->viewToPersp._13 = 0.0f;
			pC->viewToPersp._14 = 0.0f;
	pC->viewToPersp._21 = 0.0f; pC->viewToPersp._22 = 2.0f*tan(hV/2.0f);
		pC->viewToPersp._23 = 0.0f; pC->viewToPersp._24 = 0.0f;
	pC->viewToPersp._31 = 0.0f; pC->viewToPersp._32 = 0.0f;
		pC->viewToPersp._33 = 1.0f; pC->viewToPersp._34 = 0.0f;
	pC->viewToPersp._41 = 0.0f; pC->viewToPersp._42 = 0.0f;
		pC->viewToPersp._43 = 0.0f; pC->viewToPersp._44 = 1.0f;


	pC->camPos.x = pC->camPos.y = pC->camPos.z = 0.0f;

	rt_matrix4_inverse( &(pC->world), &(pC->worldInverse) );
	rt_matrix4_transpose( &(pC->worldInverse), &(pC->worldInvTr) );
	rt_matrix4_inverse( &(pC->viewToPersp), &(pC->viewToPerspInvTr) );
	rt_matrix4_transpose( &(pC->viewToPerspInvTr),
		&(pC->viewToPerspInvTr) );
}

void rt_camera_set_screen_space_matrix( rt_camera *pC, rt_matrix3 *pM )
{
	if ( (pC == NULL) || (pM == NULL) )
		exit ( -1 );

	pC->dcToSsc = *pM;
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
