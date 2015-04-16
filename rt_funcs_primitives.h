#ifndef RT_FUNCS_PRIMITIVES_H
#define RT_FUNCS_PRIMITIVES_H

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "rt_types.h"
#include "rt_funcs_math.h"

void rt_sphere_create( rt_sphere *pS, rt_vector3 *pP, 
	rt_float r, unsigned long int mat );

void rt_plane_create( rt_plane *pPl, rt_vector3 *pP, 
	rt_vector3 *pN, unsigned long int mat );

void rt_material_create( rt_material *pM, rt_color *pC, rt_color *pA,
	rt_color *pD, rt_color *pS, rt_color *pR, 
	rt_float rFoff, rt_float oD );

void rt_point_light_create( rt_point_light *pS, 
	rt_vector3 *pP, rt_float r, rt_color *pCol );

void rt_camera_create( rt_camera *pC, rt_float asp, rt_float hV );

void rt_camera_set_view_matrix( rt_camera *pC, rt_matrix4 *pM );

void rt_camera_set_perspective_matrix( rt_camera *pC, rt_matrix4 *pM );

#endif
