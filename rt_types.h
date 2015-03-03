#ifndef RT_TYPES_H
#define RT_TYPES_H

#include <CL/cl.h>

#define PRIMS_BLOCK_SIZE          33554432
#define PRIMS_IN_BLOCK            1048576
#define LIGHTS_IN_BLOCK           32
#define LIGHTS_BLOCK_SIZE         1024
#define TRIANGLES_IN_BLOCK        1048576
#define VERTEX_IN_BLOCK           1048576
#define MATERIALS_IN_BLOCK        128

#define RT_CL_RAYTRACE_PATH       "./Build/Release/CL/rt_raytrace.cl"
#define RT_CL_RAYTRACE_ARGS       "-I /home/qwerty/RayTracing"

#define SAH_PARTS                 30
#define MAX_PRIMS_IN_NODE         7
#define MAX_DEPTH                 6

typedef enum _RT_PRIMITIVE_TYPE
{
	RT_PT_SPHERE,
	RT_PT_PLANE,
	RT_PT_TRIANGLE,
} RT_PRIMITIVE_TYPE;

typedef enum _RT_AXIS
{
	RT_AXIS_X,
	RT_AXIS_Y,
	RT_AXIS_Z
} RT_AXIS;

typedef enum _RT_LIGHT_TYPE
{
	RT_LT_POINT,
	RT_LT_DIRECTED
} RT_LIGHT_TYPE;

typedef cl_float rt_float;
typedef cl_ulong rt_ulong;
typedef cl_int rt_int;

typedef struct _rt_matrix4
{
	rt_float _11, _12, _13, _14,
		_21, _22, _23, _24,
		_31, _32, _33, _34,
		_41, _42, _43, _44;
} rt_matrix4;

typedef rt_matrix4 rt_matrix3;

typedef cl_float2 rt_vector2;
typedef cl_float3 rt_vector3;
typedef cl_float4 rt_vector4;

typedef struct _rt_vecticle
{
	rt_vector3 pos;
	rt_vector3 norm;
} rt_verticle;

typedef struct _rt_ray
{
	rt_vector3 src;
	rt_vector3 dest;
	rt_vector3 invDest;
} rt_ray;

typedef struct _rt_camera
{
	rt_matrix4 world;
	rt_matrix4 worldInverse;
	rt_matrix4 viewToPersp;
	rt_matrix4 viewToPerspInvTr;
	rt_matrix4 worldInvTr;

	rt_matrix3 dcToSsc;
	rt_vector3 camPos;
	
	rt_float farZ;
	rt_float hView, wView;
	rt_float aspect;
} rt_camera;

typedef struct _rt_argb
{
	unsigned char b, g, r, a;
} rt_argb;

typedef struct _rt_color
{
	rt_float b, g, r, a;
} rt_color;

typedef struct _rt_material
{
	rt_color color;
	rt_color ambient;
	rt_color diffuse;
	rt_color specular;
	rt_color reflect;
	rt_float optDens;
} rt_material;

typedef struct _rt_sphere
{
	rt_vector3 pos;
	rt_float rad;	
	rt_ulong mat;
} rt_sphere;

typedef struct _rt_plane
{
	rt_vector3 pos;
	rt_vector3 norm;
	rt_ulong mat;
} rt_plane;

typedef struct _rt_triangle
{
	rt_ulong pV0;
	rt_ulong pV1;
	rt_ulong pV2;
	rt_ulong mat;
} rt_triangle;

typedef struct _rt_line
{
	rt_vector3 p0;
	rt_vector3 p1;
	float width;
	rt_ulong mat;
} rt_line;

typedef struct _rt_box
{
	rt_vector3 center;
	rt_vector3 extents;
} rt_box;

typedef struct _rt_point_light
{
	rt_color col;
	rt_vector3 pos;
	rt_float rad;
} rt_point_light;

typedef struct _rt_prim_desc
{
	RT_PRIMITIVE_TYPE pt;
	void *offset;
} rt_prim_desc;

typedef struct _rt_light_desc
{
	RT_LIGHT_TYPE lt;
	void *offset;
} rt_light_desc;

typedef struct _rt_cl_raytrace_args
{	
	rt_box boundingBox;
	rt_color fillCol;
	rt_ulong primsCount;
	rt_ulong trianglesCount;
	rt_ulong vertexCount;
	rt_ulong lightsCount;
	rt_int w;
	rt_int h;
	rt_int xdelta;
	rt_int ydelta;
} rt_cl_raytrace_args;

typedef struct _rt_cl_prim_desc
{
	rt_ulong offset;
	RT_PRIMITIVE_TYPE pt;
} rt_cl_prim_desc;

typedef struct _rt_cl_light_desc
{
	rt_ulong offset;
	RT_LIGHT_TYPE lt;
} rt_cl_light_desc;

typedef struct _rt_cl_kdtree_node
{
	rt_ulong leftNode;
	rt_ulong rightNode;
	rt_ulong primsCount;
	rt_ulong prims;
	float sep;
	RT_AXIS axis;	
	rt_int isLast;
} rt_cl_kdtree_node;

typedef struct _rt_kdtree_count_info
{
	rt_ulong primsCount;
	rt_ulong nodesCount;
} rt_kdtree_count_info;

typedef struct _rt_kdtree_node
{
	float sep;
	RT_AXIS axis;
	struct _rt_kdtree_node *leftNode;
	struct _rt_kdtree_node *rightNode;
	rt_ulong primsCount;
	rt_ulong *prims;
	int isLast;
} rt_kdtree_node;

typedef struct _rt_opencl_content
{
	cl_platform_id plID[5];
	cl_uint plCount;
	cl_device_id devID[5][5];
	cl_uint devCount[5];
	cl_uint computeUnitsCount;
	cl_context context;
	cl_command_queue commQue;
	cl_program prog;
	cl_kernel raytrace;
} rt_opencl_content;

typedef struct _rt_render_pipe
{	
	rt_argb *screenData;

	rt_camera *cam;

	cl_mem memp;
	cl_mem mempdecs;
	size_t primsEnd;
	rt_ulong primsCount;

	cl_mem meml;
	cl_mem memldecs;
	size_t lightsEnd;
	rt_ulong lightsCount;

	cl_mem memt;
	rt_ulong trianglesCount;

	cl_mem memv;
	size_t vertexCount;

	cl_mem memm;

	cl_mem memi;
	cl_mem memn;

	rt_opencl_content oclContent;

	rt_box boundingBox;

	rt_color fillCol;
	int w;
	int h;
} rt_render_pipe;

#endif
