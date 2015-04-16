#ifndef RT_FUNCS_MATH
#define RT_FUNCS_MATH

#include <math.h>
#include <string.h>
#include <stdlib.h>

#include "rt_types.h"

void rt_vector3_create( rt_vector3 *pV, rt_float x, 
	rt_float y, rt_float z );

void rt_color_create( rt_color *pC, 
	rt_float a, rt_float r, rt_float g, rt_float b );

void rt_matrix3_create_scale( rt_matrix3 *pR, 
	rt_float x, rt_float y, rt_float z );

void rt_matrix3_create_rotate( rt_matrix3 *pR, rt_float ang, 
	RT_AXIS a );

void rt_matrix4_create_translate( rt_matrix4 *pR, 
	rt_float x, rt_float y, rt_float z );

void rt_matrix4_create_scale( rt_matrix4 *pR, 
	rt_float x, rt_float y, rt_float z );

void rt_matrix4_create_rotate( rt_matrix4 *pR, rt_float ang, 
	RT_AXIS a );

void rt_matrix4_create_projection( rt_matrix4 *pR, rt_float asp, rt_float hV );

inline void rt_vector3_normalize( rt_vector3 *pV, rt_vector3 *pR );

inline rt_float rt_vector3_distance( rt_vector3 *pV0, rt_vector3 *pV1 );

inline rt_float rt_vector3_distance_quad( rt_vector3 *pV0, rt_vector3 *pV1 );

inline void rt_vector3_scalar_mult( rt_vector3 *pV, rt_float s, rt_vector3 *pR );

inline void rt_vector3_sub( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *rR );

inline void rt_vector3_add( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *pR );

inline rt_float rt_vector3_length( rt_vector3 *pV );

inline rt_float rt_vector3_length_quad( rt_vector3 *pV );


inline rt_float rt_vector3_dot( rt_vector3 *pV0, rt_vector3 *pV1 );

inline void rt_vector3_cross( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *pR );

inline void rt_vector3_reflect( rt_vector3 *pV, rt_vector3 *pN, rt_vector3 *pR );

inline void rt_vector3_matrix3_mult( rt_vector3 *pV, rt_matrix3 *pM, rt_vector3 *pR );

inline void rt_vector3_matrix4_mult( rt_vector3 *pV, rt_matrix4 *pM, rt_vector3 *pR );

inline void rt_vector3_matrix4_mult_dir( rt_vector3 *pV, rt_matrix4 *pM, 
	rt_vector3 *pR );

inline void rt_color_clamp( rt_color *pC, rt_color *pR );

inline void rt_color_mult( rt_color *pC0, rt_color *pC1,
	rt_color *pR );

inline void rt_color_add( rt_color *pC0, rt_color *pC1,
	rt_color *pR );

inline void rt_color_scalar_mult( rt_color *pC, rt_float s,
	rt_color *pR );

inline void rt_matrix4_inverse( rt_matrix4 *pM, rt_matrix4 *pR );

inline void rt_matrix4_transpose( rt_matrix4 *pM, rt_matrix4 *pR );

inline void rt_matrix3_inverse( rt_matrix3 *pM, rt_matrix3 *pR );

inline void rt_matrix3_transpose( rt_matrix3 *pM, rt_matrix3 *pR );

inline void rt_matrix4_mult( rt_matrix4 *pM0, rt_matrix4 *pM1, rt_matrix4 *pR );

inline void rt_matrix3_mult( rt_matrix3 *pM0, rt_matrix3 *pM1, rt_matrix3 *pR );

inline rt_float rt_clamp_float( rt_float f, rt_float b, rt_float e );

inline rt_float minF( rt_float a, rt_float b );

inline rt_float maxF( rt_float a, rt_float b );

inline int is_power_of_two_size_t( size_t a );

#endif
