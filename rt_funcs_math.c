#include "rt_funcs_math.h"

void rt_vector3_create( rt_vector3 *pV, rt_float x, 
	rt_float y, rt_float z )
{
	if ( pV == NULL )
		exit( -1 );

	pV->x = x;
	pV->y = y;
	pV->z = z;
}

void rt_color_create( rt_color *pC, 
	rt_float a, rt_float r, rt_float g, rt_float b )
{
	if ( pC == NULL )
		exit( -1 );

	pC->a = a;
	pC->r = r;
	pC->g = g;
	pC->b = b;
}

void rt_matrix3_create_scale( rt_matrix3 *pR, 
	rt_float x, rt_float y, rt_float z )
{
	if ( pR == NULL )
		exit( -1 );

	pR->_11 = x;    pR->_12 = 0.0f; pR->_13 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = y;    pR->_23 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f; pR->_33 = z;
}

void rt_matrix3_create_rotate( rt_matrix3 *pR, rt_float ang, RT_AXIS a )
{
	if ( pR == NULL )
		exit( -1 );

	switch ( a )
	{
	case RT_AXIS_X:
		pR->_11 = 1.0f; pR->_12 = 0.0f;      pR->_13 = 0.0f;
		pR->_21 = 0.0f; pR->_22 = cos(ang);  pR->_23 = sin(ang);
		pR->_31 = 0.0f; pR->_32 = -sin(ang); pR->_33 = cos(ang);
		break;

	case RT_AXIS_Y:
		pR->_11 = cos(ang); pR->_12 = 0.0f;     pR->_13 = -sin(ang);
		pR->_21 = 0.0f;     pR->_22 = 1.0f;     pR->_23 = 0.0f;
		pR->_31 = sin(ang); pR->_32 = 0.0f;     pR->_33 = cos(ang);
		break;
	default:
		break;
	}
}

void rt_matrix4_create_translate( rt_matrix4 *pR,
	rt_float x, rt_float y, rt_float z )
{
	if ( pR == NULL )
		exit( -1 );

	pR->_11 = 1.0f; pR->_12 = 0.0f; pR->_13 = 0.0f; pR->_14 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = 1.0f; pR->_23 = 0.0f; pR->_24 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f; pR->_33 = 1.0f; pR->_34 = 0.0f;
	pR->_41 = x;    pR->_42 = y;    pR->_43 = z;    pR->_44 = 1.0f;
}


void rt_matrix4_create_scale( rt_matrix4 *pR, 
	rt_float x, rt_float y, rt_float z )
{
	if ( pR == NULL )
		exit( -1 );

	pR->_11 = x;    pR->_12 = 0.0f; pR->_13 = 0.0f; pR->_14 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = y;    pR->_23 = 0.0f; pR->_24 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f; pR->_33 = z;    pR->_34 = 0.0f;
	pR->_41 = 0.0f; pR->_42 = 0.0f; pR->_43 = 0.0f; pR->_44 = 1.0f;
}

void rt_matrix4_create_rotate( rt_matrix4 *pR, rt_float ang, RT_AXIS a )
{
	if ( pR == NULL )
		exit( -1 );

	switch ( a )
	{
	case RT_AXIS_X:
		pR->_11 = 1.0f; pR->_12 = 0.0f;      
			pR->_13 = 0.0f; pR->_14 = 0.0f;
		pR->_21 = 0.0f; pR->_22 = cos(ang);
			pR->_23 = sin(ang); pR->_24 = 0.0f;
		pR->_31 = 0.0f; pR->_32 = -sin(ang);
			pR->_33 = cos(ang); pR->_34 = 0.0f;
		pR->_41 = 0.0f; pR->_42 = 0.0f;
			pR->_43 = 0.0f; pR->_44 = 1.0f;
		break;

	case RT_AXIS_Y:
		pR->_11 = cos(ang); pR->_12 = 0.0f;
			pR->_13 = -sin(ang); pR->_14 = 0.0f;
		pR->_21 = 0.0f; pR->_22 = 1.0f;
			pR->_23 = 0.0f; pR->_24 = 0.0f;
		pR->_31 = sin(ang); pR->_32 = 0.0f; 
			pR->_33 = cos(ang); pR->_34 = 0.0f;
		pR->_41 = 0.0f; pR->_42 = 0.0f;
			pR->_43 = 0.0f; pR->_44 = 1.0f;
		break;

	case RT_AXIS_Z:
		pR->_11 = cos(ang); pR->_12 = sin(ang);
			pR->_13 = 0.0f; pR->_14 = 0.0f;
		pR->_21 = -sin(ang); pR->_22 = cos(ang);
			pR->_23 = 0.0f; pR->_24 = 0.0f;
		pR->_31 = 0.0f; pR->_32 = 0.0f; 
			pR->_33 = 1.0f; pR->_34 = 0.0f;
		pR->_41 = 0.0f; pR->_42 = 0.0f;
			pR->_43 = 0.0f; pR->_44 = 1.0f;
		break;
	}
}

void rt_matrix4_create_projection( rt_matrix4 *pR, rt_float asp, rt_float hV )
{
	pR->_11 = 2.0f*tan(hV/2.0f)*asp; pR->_12 = 0.0f;
		pR->_13 = 0.0f; pR->_14 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = 2.0f*tan(hV/2.0f);
		pR->_23 = 0.0f; pR->_24 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f;
		pR->_33 = 1.0f; pR->_34 = 0.0f;
	pR->_41 = 0.0f; pR->_42 = 0.0f;
		pR->_43 = 0.0f; pR->_44 = 1.0f;
}

inline rt_float rt_clamp_float( rt_float f, rt_float b, rt_float e )
{
	return (f > b) ? ((f < e) ? f : e) : b;
}

inline void rt_vector3_normalize( rt_vector3 *pV, rt_vector3 *pR )
{
	rt_float tmp;

	tmp = sqrt(pV->x * pV->x + pV->y * pV->y + pV->z * pV->z);
	
	pR->x = pV->x / tmp;
	pR->y = pV->y / tmp;
	pR->z = pV->z / tmp;
}

// distance between two points
inline rt_float rt_vector3_distance( rt_vector3 *pV0, rt_vector3 *pV1 )
{
	return sqrt((pV0->x-pV1->x) * (pV0->x-pV1->x)
		+ (pV0->y-pV1->y) * (pV0->y-pV1->y)
		+ (pV0->z-pV1->z) * (pV0->z-pV1->z) );
}

// square of distance between two points
inline rt_float rt_vector3_distance_quad( rt_vector3 *pV0, rt_vector3 *pV1 )
{
	return (pV0->x-pV1->x) * (pV0->x-pV1->x)
		+ (pV0->y-pV1->y) * (pV0->y-pV1->y)
		+ (pV0->z-pV1->z) * (pV0->z-pV1->z);
}

// multiply vector by scalar
void rt_vector3_scalar_mult( rt_vector3 *pV, rt_float s, rt_vector3 *pR )
{
	pR->x = pV->x * s;
	pR->y = pV->y * s;
	pR->z = pV->z * s;
}

inline void rt_vector3_sub( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *r )
{
	r->x = pV1->x - pV0->x;
	r->y = pV1->y - pV0->y;
	r->z = pV1->z - pV0->z;
}

inline void rt_vector3_add( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *pR )
{
	pR->x = pV1->x + pV0->x;
	pR->y = pV1->y + pV0->y;
	pR->z = pV1->z + pV0->z;
}

inline rt_float rt_vector3_length( rt_vector3 *pV )
{
	return sqrt( pV->x*pV->x + pV->y*pV->y + pV->z*pV->z );
}

inline rt_float rt_vector3_length_quad( rt_vector3 *pV )
{
	return (pV->x*pV->x + pV->y*pV->y + pV->z*pV->z);
}

void rt_vector3_matrix3_mult( rt_vector3 *pV, rt_matrix3 *pM, rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV->x + pM->_21 * pV->y + pM->_31 * pV->z;
	tmp.y = pM->_12 * pV->x + pM->_22 * pV->y + pM->_32 * pV->z;
	tmp.z = pM->_13 * pV->x + pM->_23 * pV->y + pM->_33 * pV->z;

	memcpy( pR, &tmp, sizeof(rt_vector3) );

}

inline rt_float rt_vector3_dot( rt_vector3 *pV0, rt_vector3 *pV1 )
{
	return (pV0->x * pV1->x + pV0->y * pV1->y + pV0->z * pV1->z);
}

inline void rt_vector3_cross( rt_vector3 *pV0, rt_vector3 *pV1, rt_vector3 *pR )
{
	if ( (pR == pV0) || (pR == pV1) )
	{	
		rt_vector3 tmp;
		tmp.x = (pV0->y) * (pV1->z) - (pV0->z) * (pV1->y);
		tmp.y = (pV0->z) * (pV1->x) - (pV0->x) * (pV1->z);
		tmp.z = (pV0->x) * (pV1->y) - (pV0->y) * (pV1->x);

		memcpy( pR, &tmp, sizeof(rt_vector3) ); 
	}
	else
	{
		pR->x = (pV0->y) * (pV1->z) - (pV0->z) * (pV1->y);
		pR->y = (pV0->z) * (pV1->x) - (pV0->x) * (pV1->z);
		pR->z = (pV0->x) * (pV1->y) - (pV0->y) * (pV1->x);
	}
}

// reflect vector by normal
inline void rt_vector3_reflect( rt_vector3 *pV, rt_vector3 *pN, rt_vector3 *pR )
{
	rt_vector3 tmpVec;

	rt_vector3_scalar_mult( pN, 2.0f * rt_vector3_dot( pV, pN ), &tmpVec );
	rt_vector3_sub( &tmpVec, pV, pR );
}

inline void rt_vector3_matrix4_mult( rt_vector3 *pV, rt_matrix4 *pM, rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV->x + pM->_21 * pV->y + pM->_31 * pV->z + pM->_41;
	tmp.y = pM->_12 * pV->x + pM->_22 * pV->y + pM->_32 * pV->z + pM->_42;
	tmp.z = pM->_13 * pV->x + pM->_23 * pV->y + pM->_33 * pV->z + pM->_43;

	memcpy( pR, &tmp, sizeof(rt_vector3) );
}

inline void rt_vector3_matrix4_mult_dir( rt_vector3 *pV, rt_matrix4 *pM, 
	rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV->x + pM->_21 * pV->y + pM->_31 * pV->z;
	tmp.y = pM->_12 * pV->x + pM->_22 * pV->y + pM->_32 * pV->z;
	tmp.z = pM->_13 * pV->x + pM->_23 * pV->y + pM->_33 * pV->z;

	memcpy( pR, &tmp, sizeof(rt_vector3) );
}

inline void rt_color_clamp( rt_color *pC, rt_color *pR )
{
	pR->a = rt_clamp_float( pC->a, 0.0f, 1.0f );
	pR->r = rt_clamp_float( pC->r, 0.0f, 1.0f );
	pR->g = rt_clamp_float( pC->g, 0.0f, 1.0f );
	pR->b = rt_clamp_float( pC->b, 0.0f, 1.0f );
}

inline void rt_color_mult( rt_color *pC0, rt_color *pC1,
	rt_color *pR )
{
	pR->a = pC0->a * pC1->a;
	pR->r = pC0->r * pC1->r;
	pR->g = pC0->g * pC1->g;
	pR->b = pC0->b * pC1->b;

	rt_color_clamp( pR, pR );
}

inline void rt_color_add( rt_color *pC0, rt_color *pC1,
	rt_color *pR )
{
	pR->a = pC0->a + pC1->a;
	pR->r = pC0->r + pC1->r;
	pR->g = pC0->g + pC1->g;
	pR->b = pC0->b + pC1->b;

	rt_color_clamp( pR, pR );
}

inline void rt_color_scalar_mult( rt_color *pC, rt_float s,
	rt_color *pR )
{
	pR->a = pC->a*s;
	pR->r = pC->r*s;
	pR->g = pC->g*s;
	pR->b = pC->b*s;

	rt_color_clamp( pR, pR );
}

inline void rt_matrix4_inverse( rt_matrix4 *pM, rt_matrix4 *pR )
{
	rt_float invM[4][4], tmpM[4][4];
	rt_float coef;
	int i, j, k;

	memcpy( tmpM, pM, sizeof(rt_matrix4) );

	for ( i = 0; i < 4; ++i )
		for ( j = 0; j < 4; ++j )
			invM[i][j] = (i == j) ? 1.0f : 0.0;

	for ( k = 0; k < 4; ++k )
	{
		coef = 1/tmpM[k][k];
		for ( j = 0; j < 4; ++j )
		{
			tmpM[k][j] *= coef;
			invM[k][j] *= coef;
		}


		for ( i = 0; i < 4; ++i )
			if ( i != k )
			{
				coef = -(tmpM[i][k] / tmpM[k][k]);
				for ( j = 0; j < 4; ++j )
				{
					tmpM[i][j] += tmpM[k][j]*coef;
					invM[i][j] += invM[k][j]*coef;
				} 
			}			
	}
	memcpy( pR, invM, sizeof(rt_matrix4) );
}

inline void rt_matrix4_transpose( rt_matrix4 *pM, rt_matrix4 *pR )
{
	rt_float tmp;

	tmp = pM->_12; pR->_12 = pM->_21; pR->_21 = tmp;	
	tmp = pM->_13; pR->_13 = pM->_31; pR->_31 = tmp;	
	tmp = pM->_14; pR->_14 = pM->_41; pR->_41 = tmp;	

	tmp = pM->_23; pR->_23 = pM->_32; pR->_32 = tmp;	
	tmp = pM->_24; pR->_24 = pM->_42; pR->_42 = tmp;	

	tmp = pM->_34; pR->_34 = pM->_43; pR->_43 = tmp;

	pR->_44 = pM->_44;
}

inline void rt_matrix3_inverse( rt_matrix3 *pM, rt_matrix3 *pR )
{
	rt_float invM[3][3], tmpM[3][3];
	rt_float coef;
	int i, j, k;

	memcpy( tmpM, pM, sizeof(rt_matrix3) );

	for ( i = 0; i < 3; ++i )
		for ( j = 0; j < 3; ++j )
			invM[i][j] = (i == j) ? 1.0f : 0.0;

	for ( k = 0; k < 3; ++k )
	{
		coef = 1/tmpM[k][k];
		for ( j = 0; j < 3; ++j )
		{
			tmpM[k][j] *= coef;
			invM[k][j] *= coef;
		}


		for ( i = 0; i < 3; ++i )
			if ( i != k )
			{
				coef = -(tmpM[i][k] / tmpM[k][k]);
				for ( j = 0; j < 3; ++j )
				{
					tmpM[i][j] += tmpM[k][j]*coef;
					invM[i][j] += invM[k][j]*coef;
				} 
			}			
	}
	memcpy( pR, invM, sizeof(rt_matrix4) );
}

inline void rt_matrix3_transpose( rt_matrix3 *pM, rt_matrix3 *pR )
{
	rt_float tmp;

	tmp = pM->_12; pR->_12 = pM->_21; pR->_21 = tmp;	
	tmp = pM->_13; pR->_13 = pM->_31; pR->_31 = tmp;	

	tmp = pM->_23; pR->_23 = pM->_32; pR->_32 = tmp;	
}

inline void rt_matrix4_mult( rt_matrix4 *pM0, rt_matrix4 *pM1, rt_matrix4 *pR )
{
	rt_matrix4 tmp;

	tmp._11 = (pM1->_11 * pM0->_11) + (pM1->_21 * pM0->_12)
		+ (pM1->_31 * pM0->_13) + (pM1->_41 * pM0->_14);
	tmp._12 = (pM1->_12 * pM0->_11) + (pM1->_22 * pM0->_12)
		+ (pM1->_32 * pM0->_13) + (pM1->_42 * pM0->_14);
	tmp._13 = (pM1->_13 * pM0->_11) + (pM1->_23 * pM0->_12)
		+ (pM1->_33 * pM0->_13) + (pM1->_43 * pM0->_14);
	tmp._14 = (pM1->_14 * pM0->_11) + (pM1->_24 * pM0->_12)
		+ (pM1->_34 * pM0->_13) + (pM1->_44 * pM0->_14);

	tmp._21 = (pM1->_11 * pM0->_21) + (pM1->_21 * pM0->_22)
		+ (pM1->_31 * pM0->_23) + (pM1->_41 * pM0->_24);
	tmp._22 = (pM1->_12 * pM0->_21) + (pM1->_22 * pM0->_22)
		+ (pM1->_32 * pM0->_23) + (pM1->_42 * pM0->_24);
	tmp._23 = (pM1->_13 * pM0->_21) + (pM1->_23 * pM0->_22)
		+ (pM1->_33 * pM0->_23) + (pM1->_43 * pM0->_24);
	tmp._24 = (pM1->_14 * pM0->_21) + (pM1->_24 * pM0->_22)
		+ (pM1->_34 * pM0->_23) + (pM1->_44 * pM0->_24);

	tmp._31 = (pM1->_11 * pM0->_31) + (pM1->_21 * pM0->_32)
		+ (pM1->_31 * pM0->_33) + (pM1->_41 * pM0->_34);
	tmp._32 = (pM1->_12 * pM0->_31) + (pM1->_22 * pM0->_32)
		+ (pM1->_32 * pM0->_33) + (pM1->_42 * pM0->_34);
	tmp._33 = (pM1->_13 * pM0->_31) + (pM1->_23 * pM0->_32)
		+ (pM1->_33 * pM0->_33) + (pM1->_43 * pM0->_34);
	tmp._34 = (pM1->_14 * pM0->_31) + (pM1->_24 * pM0->_32)
		+ (pM1->_34 * pM0->_33) + (pM1->_44 * pM0->_34);

	tmp._41 = (pM1->_11 * pM0->_41) + (pM1->_21 * pM0->_42)
		+ (pM1->_31 * pM0->_43) + (pM1->_41 * pM0->_44);
	tmp._42 = (pM1->_12 * pM0->_41) + (pM1->_22 * pM0->_42)
		+ (pM1->_32 * pM0->_43) + (pM1->_42 * pM0->_44);
	tmp._43 = (pM1->_13 * pM0->_41) + (pM1->_23 * pM0->_42)
		+ (pM1->_33 * pM0->_43) + (pM1->_43 * pM0->_44);
	tmp._44 = (pM1->_14 * pM0->_41) + (pM1->_24 * pM0->_42)
		+ (pM1->_34 * pM0->_43) + (pM1->_44 * pM0->_44);

	memcpy( pR, &tmp, sizeof(rt_matrix4) );
}

inline void rt_matrix3_mult( rt_matrix3 *pM0, rt_matrix3 *pM1, rt_matrix3 *pR )
{
	rt_matrix3 tmp;

	tmp._11 = (pM1->_11 * pM0->_11) + (pM1->_21 * pM0->_12)
		+ (pM1->_31 * pM0->_13);
	tmp._12 = (pM1->_12 * pM0->_11) + (pM1->_22 * pM0->_12)
		+ (pM1->_32 * pM0->_13);
	tmp._13 = (pM1->_13 * pM0->_11) + (pM1->_23 * pM0->_12)
		+ (pM1->_33 * pM0->_13);

	tmp._21 = (pM1->_11 * pM0->_21) + (pM1->_21 * pM0->_22)
		+ (pM1->_31 * pM0->_23);
	tmp._22 = (pM1->_12 * pM0->_21) + (pM1->_22 * pM0->_22)
		+ (pM1->_32 * pM0->_23);
	tmp._23 = (pM1->_13 * pM0->_21) + (pM1->_23 * pM0->_22)
		+ (pM1->_33 * pM0->_23);

	tmp._31 = (pM1->_11 * pM0->_31) + (pM1->_21 * pM0->_32)
		+ (pM1->_31 * pM0->_33);
	tmp._32 = (pM1->_12 * pM0->_31) + (pM1->_22 * pM0->_32)
		+ (pM1->_32 * pM0->_33);
	tmp._33 = (pM1->_13 * pM0->_31) + (pM1->_23 * pM0->_32)
		+ (pM1->_33 * pM0->_33);

	memcpy( pR, &tmp, sizeof(rt_matrix3) );
}

inline rt_float minF( rt_float a, rt_float b )
{
	return (a < b) ? a : b;
}

inline rt_float maxF( rt_float a, rt_float b )
{
	return (a > b) ? a : b;
}

inline int is_power_of_two_size_t( size_t a )
{
	size_t mask = 1;
	int counter = 0;

	while ( mask != 0 )
	{
		if ( a & mask )
		{
			++counter;

			if ( counter > 1 )
				return 0;
		}

		mask <<= 1;
	}

	return 1;
}
