#ifndef RT_FUNCS_RENDER_PIPE_H
#define RT_FUNCS_RENDER_PIPE_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rt_types.h"
#include "rt_funcs_primitives.h"
#include "rt_funcs_math.h"

char *rt_cl_include_path;
char *rt_cl_raytrace_kernel_path;

void rt_init( char *path );

void rt_render_pipe_create( rt_render_pipe *pRp, int w, 
	int h, void *sD );

void rt_init_buffers( rt_render_pipe *pRp );

void rt_render_pipe_set_camera( rt_render_pipe *pRp, 
	rt_camera *pFr );

void rt_render_pipe_get_camera( rt_render_pipe *pRp, rt_camera **ppC );

void rt_render_pipe_add_primitive( rt_render_pipe *pRp, void *pPrim,
	RT_PRIMITIVE_TYPE pt );

void rt_render_pipe_add_triangles( rt_render_pipe *pRp, rt_verticle *pV, 
	rt_triangle *pTr, int vCount, int tCount );

void rt_render_pipe_add_material( rt_render_pipe *pRp, rt_material *mat, 
	unsigned long int ind ); 

void rt_render_pipe_add_light( rt_render_pipe *pRp, void *pLight,
	RT_LIGHT_TYPE pt );

void rt_init_opencl( rt_render_pipe *pRp );

rt_kdtree_count_info rt_kdtree_make_childs( rt_verticle *pV, rt_triangle *pTr, 
	rt_kdtree_node *pNode, rt_box *bbox, 
	int depth );

void rt_kdtree_compute_sah( rt_ulong *memTp, rt_triangle *memTr, 
	rt_verticle *memVer, rt_ulong primsCount, RT_AXIS axis, rt_box *pBox, 
	rt_ulong *primsCountL, rt_ulong *primsCountR, float *sep );


rt_kdtree_count_info rt_kdtree_pack_to_buffer( rt_cl_kdtree_node *pNodeBuf, 
	rt_ulong *pPrimsIdxBuf, rt_kdtree_node *pNode, 
	rt_ulong writePos, rt_ulong primsWritePos );

void rt_kdtree_build( rt_render_pipe *pRp );

rt_argb *rt_render_pipe_draw( rt_render_pipe *pRp );

void rt_cleanup( rt_render_pipe *pRp );

#endif
