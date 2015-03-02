#ifndef RT_FUNCS_MATH_CL
#define RT_FUNCS_MATH_CL

#include "CL/rt_types.cl"

void rt_color_create( rt_color *pC, 
	float a, float r, float g, float b )
{
	pC->a = a;
	pC->r = r;
	pC->g = g;
	pC->b = b;
}

void rt_matrix4_create_rotate( rt_matrix4 *pR, float ang, 
	RT_AXIS a )
{
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

void rt_matrix4_create_translate( rt_matrix4 *pR, 
float x, float y, float z )
{
	pR->_11 = 1.0f; pR->_12 = 0.0f; pR->_13 = 0.0f; pR->_14 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = 1.0f; pR->_23 = 0.0f; pR->_24 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f; pR->_33 = 1.0f; pR->_34 = 0.0f;
	pR->_41 = x;    pR->_42 = y;    pR->_43 = z;    pR->_44 = 1.0f;
}

void rt_matrix3_create_translate( rt_matrix3 *pR, 
	float x, float y )
{
	pR->_11 = 1.0f; pR->_12 = 0.0f; pR->_13 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = 1.0f; pR->_23 = 0.0f;
	pR->_31 = x; pR->_32 = y; pR->_33 = 1.0f;

}

void rt_matrix3_create_scale( rt_matrix3 *pR, 
	float x, float y )
{
	pR->_11 = x;    pR->_12 = 0.0f; pR->_13 = 0.0f;
	pR->_21 = 0.0f; pR->_22 = y;    pR->_23 = 0.0f;
	pR->_31 = 0.0f; pR->_32 = 0.0f; pR->_33 = 1.0f;
}

void rt_matrix3_create_rotate( rt_matrix3 *pR, float ang, 
	RT_AXIS a )
{
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

inline float rt_clamp_float( float f, float b, float e )
{
	return (f > b) ? ((f < e) ? f : e) : b;
}

void rt_vector3_matrix3_mult( rt_vector3 pV, rt_matrix3 *pM, rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV.x + pM->_21 * pV.y + pM->_31 * pV.z;
	tmp.y = pM->_12 * pV.x + pM->_22 * pV.y + pM->_32 * pV.z;
	tmp.z = pM->_13 * pV.x + pM->_23 * pV.y + pM->_33 * pV.z;

	*pR = tmp;	
}

// reflect vector by normal
inline rt_vector3 rt_vector3_reflect( rt_vector3 pV, rt_vector3 pN )
{
/*	rt_vector3 tmpVec;

	rt_vector3_scalar_mult( pN, 2.0f * rt_vector3_dot( pV, pN ), &tmpVec );
	rt_vector3_sub( &tmpVec, pV, pR );
*/
	rt_vector3 tmpVec = (pV - pN * 2.0f * dot( pV, pN ));

	return tmpVec;
}

void rt_vector3_matrix4_mult( rt_vector3 pV, rt_matrix4 *pM, rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV.x + pM->_21 * pV.y + pM->_31 * pV.z + pM->_41;
	tmp.y = pM->_12 * pV.x + pM->_22 * pV.y + pM->_32 * pV.z + pM->_42;
	tmp.z = pM->_13 * pV.x + pM->_23 * pV.y + pM->_33 * pV.z + pM->_43;

	*pR = tmp;	
}

inline void rt_vector3_matrix4_mult_dir( rt_vector3 pV, rt_matrix4 *pM, 
	rt_vector3 *pR )
{
	rt_vector3 tmp;

	tmp.x = pM->_11 * pV.x + pM->_21 * pV.y + pM->_31 * pV.z;
	tmp.y = pM->_12 * pV.x + pM->_22 * pV.y + pM->_32 * pV.z;
	tmp.z = pM->_13 * pV.x + pM->_23 * pV.y + pM->_33 * pV.z;

	*pR = tmp;	
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

inline void rt_color_scalar_mult( rt_color *pC, float s,
	rt_color *pR )
{
	pR->a = pC->a*s;
	pR->r = pC->r*s;
	pR->g = pC->g*s;
	pR->b = pC->b*s;

	rt_color_clamp( pR, pR );
}

inline void rt_matrix4_transpose( rt_matrix4 *pM, rt_matrix4 *pR )
{
	float tmp;

	tmp = pM->_12; pR->_12 = pM->_21; pR->_21 = tmp;	
	tmp = pM->_13; pR->_13 = pM->_31; pR->_31 = tmp;	
	tmp = pM->_14; pR->_14 = pM->_41; pR->_41 = tmp;	

	tmp = pM->_23; pR->_23 = pM->_32; pR->_32 = tmp;	
	tmp = pM->_24; pR->_24 = pM->_42; pR->_42 = tmp;	

	tmp = pM->_34; pR->_34 = pM->_43; pR->_43 = tmp;

	pR->_44 = pM->_44;
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

	*pR = tmp;
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

	*pR = tmp;
}

inline void rt_vector2_matrix3_mult( rt_vector2 pV, rt_matrix3 *pM, rt_vector2 *pR )
{
	rt_vector2 tmp;

	tmp.x = (pM->_11 * pV.x) + (pM->_21 * pV.y) + pM->_31;
	tmp.y = (pM->_12 * pV.x) + (pM->_22 * pV.y) + pM->_32; 

	*pR = tmp;
}

inline void rt_vector4_matrix4_mult( rt_vector4 pV, rt_matrix4 *pM, rt_vector4 *pR )
{
	rt_vector4 tmp;

	tmp.x = pM->_11 * pV.x + pM->_21 * pV.y 
		+ pM->_31 * pV.z + pM->_41 * pV.w;
	tmp.y = pM->_12 * pV.x + pM->_22 * pV.y 
		+ pM->_32 * pV.z + pM->_42 * pV.w; 
	tmp.z = pM->_13 * pV.x + pM->_23 * pV.y 
		+ pM->_33 * pV.z + pM->_43 * pV.w;
	tmp.w = pM->_14 * pV.x + pM->_24 * pV.y 
		+ pM->_34 * pV.z + pM->_44 * pV.w;

	*pR = tmp;
}

inline float minF( float a, float b )
{
	return (a < b) ? a : b;
}

inline float maxF( float a, float b )
{
	return (a > b) ? a : b;
}

#endif
