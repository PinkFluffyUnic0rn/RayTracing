#ifndef RT_TYPES_CL
#define RT_TYPES_CL

#define STACK_SIZE            6

#define EPSILON               0.05f              

#define ENV_OPT_DENSITY       1.0f

#define SHADOWS_ENABLED       1
#define AMBIENT_ENABLED       1
#define DIFFUSE_ENABLED       1
#define SPECULAR_ENABLED      1

#define KDTREE_DEPTH          6

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

typedef struct _rt_matrix4
{
	float _11, _12, _13, _14,
		_21, _22, _23, _24,
		_31, _32, _33, _34,
		_41, _42, _43, _44;
} rt_matrix4;

typedef rt_matrix4 rt_matrix3;

typedef float2 rt_vector2;
typedef float3 rt_vector3;
typedef float4 rt_vector4;

typedef struct _rt_vertex
{
	rt_vector3 pos;
	rt_vector3 norm;
} rt_vertex;

typedef struct _rt_ray
{
	rt_vector3 src;
	rt_vector3 dest;
	rt_vector3 invDest;
} rt_ray;

typedef struct _rt_camera
{
	rt_matrix4 world;
	rt_matrix4 viewToPersp;
} rt_camera;

typedef struct _rt_argb
{
	unsigned char b, g, r, a;
} rt_argb;

typedef struct _rt_color
{
	float b;
	float g;
	float r;
	float a;
} rt_color;

typedef struct _rt_material
{
	rt_color color;
	rt_color ambient;
	rt_color diffuse;
	rt_color specular;
	rt_color reflect;
	float lightFalloff;
	float optDens;
} rt_material;

typedef struct _rt_sphere
{
	rt_vector3 pos;
	float rad;	
	ulong mat;
} rt_sphere;

typedef struct _rt_plane
{
	rt_vector3 pos;
	rt_vector3 norm;
	ulong mat;
} rt_plane;

typedef struct _rt_triangle
{
	ulong v0;
	ulong v1;
	ulong v2;
	ulong mat;
} rt_triangle;

typedef struct _rt_box
{
	rt_vector3 center;
	rt_vector3 extents;
} rt_box;

typedef struct _rt_point_light
{
	rt_color col;
	rt_vector3 pos;
	float rad;
} rt_point_light;

typedef struct _rt_raytrace_args
{	
	rt_box boundingBox;
	ulong primsCount;
	ulong trianglesCount;
	ulong vertexCount;
	ulong lightsCount;
	int w;
	int h;
	int xdelta;
	int ydelta;
} rt_raytrace_args;

typedef struct _rt_prim_desc
{
	ulong offset;
	RT_PRIMITIVE_TYPE pt;
} rt_prim_desc;

typedef struct _rt_light_desc
{
	ulong offset;
	RT_LIGHT_TYPE lt;
} rt_light_desc;

typedef struct _rt_cl_kdtree_node
{
	ulong leftNode;
	ulong rightNode;
	ulong primsCount;
	ulong prims;
	float sep;
	RT_AXIS axis;
	int isLast;
} rt_cl_kdtree_node;

typedef struct _stackElement
{
	rt_ray ray;
	rt_color col;
	rt_vector3 p;
	ulong pPrMat;
	int parentIdx;
	int bNearest;
	int type;
} stackElement;

typedef struct _rt_cl_render_pipe_data
{
	rt_box boundingBox;

	rt_color fillCol;

	__global const void *primsBuf;
	__global const rt_prim_desc *primsDecs;
	__global const void *lightsBuf;
	__global const rt_light_desc *lightsDecs;
	__global const rt_material *materialBuf;
	__global const rt_triangle *trianglesBuf;
	__global const rt_vertex *vertexBuf;
	__global const rt_cl_kdtree_node *kdtreeNodesBuf;
	__global const ulong *kdtreePrimsIdxBuf;

	ulong primsCount;
	ulong lightsCount;
	ulong trianglesCount;
	ulong vertexCount;

	int w;
	int h;	
} rt_cl_render_pipe_data;

#endif
