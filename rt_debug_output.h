#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

#include <stdio.h>
#include "rt_types.h"

void rt_dout_matrix4( rt_matrix4 *pM );
void rt_dout_vector3( rt_vector3 *pV );
void rt_dout_kdtree( rt_verticle *pV, rt_triangle *pTr, 
	rt_render_pipe *pRp, rt_kdtree_node *pNode, int depth );
void rt_cl_dout_kdtree( rt_verticle *pV, rt_triangle *pTr, rt_cl_kdtree_node *pN, 
	rt_ulong *pP, rt_render_pipe *pRp, rt_ulong nodeN, int depth );

#endif
