#ifndef RT_MESHES_H
#define RT_MESHES_H

#include "rt.h"

typedef struct _rt_mesh_group
{
	rt_material *mat;
	
	rt_ulong voffset;
	rt_ulong toffset;
	rt_ulong vc;
	rt_ulong tc;
} rt_mesh_group;

typedef struct _rt_mesh
{
	rt_vertex *v;
	rt_triangle *t;
	rt_mesh_group *g;
	rt_material *mat;
	
	rt_ulong mc;
	rt_ulong gc;
	rt_ulong vc;
	rt_ulong tc;
} rt_mesh;

int rt_mesh_load_from_obj( rt_mesh *m, char *path );
void rt_mesh_add( rt_render_pipe *pRp, rt_mesh *m,  rt_ulong tg );
void rt_mesh_add_group( rt_render_pipe *pRp, rt_mesh *m, rt_ulong g, rt_ulong tg );
void rt_mesh_reserve( rt_mesh *m, rt_ulong vc, rt_ulong tc , rt_ulong gc );
void rt_mesh_release( rt_mesh *m );

#endif
