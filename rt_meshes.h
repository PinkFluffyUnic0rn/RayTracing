#ifndef RT_MESHES_H
#define RT_MESHES_H

#include "rt.h"

typedef struct _rt_mesh
{
	rt_vertex v[300000];
	rt_triangle t[300000];
	
	rt_matrix4 toWorld;
	rt_material mat;

	int vc;
	int tc;
} rt_mesh;

int rt_mesh_load_from_obj( rt_mesh *m, char *path );
void rt_mesh_add( rt_render_pipe *pRp, rt_mesh *m );

#endif
