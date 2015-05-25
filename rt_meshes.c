#include "rt_meshes.h"
#include <fcntl.h>

int rt_mesh_load_from_obj( rt_mesh *m, char *path )
{
	FILE *file = fopen( path, "r" );
	char buf[255];

	if ( file == NULL )
		return -1;

	{
		int curV = 0, curN = 0, curT = 0;
		rt_vector3 n[300000];
		
		while ( fgets( buf, 255, file ) != NULL )
		{
			if ( strncmp( buf, "vn", 2 ) == 0 )
			{
				sscanf( buf + 2, "%f %f %f",
					&(n[curN].x),
					&(n[curN].y),
					&(n[curN].z) );

				rt_vector3_normalize( n+curN, n+curN );
				++curN;
			}
			else if ( strncmp( buf, "v", 1 ) == 0 )
			{
				sscanf( buf + 2, "%f %f %f",
					&(m->v[curV].pos.x),
					&(m->v[curV].pos.y),
					&(m->v[curV].pos.z) );
				
				++curV;
			}
			else if ( strncmp( buf, "f", 1 ) == 0 )
			{
				int vi[3];
				int ni[3];

				sscanf( buf + 2, "%d//%d %d//%d %d//%d",
					vi, ni,
					vi + 1, ni + 1,
					vi + 2, ni + 2 );
					
				m->t[curT].pV0 = vi[0]-1;
				m->t[curT].pV1 = vi[1]-1;
				m->t[curT].pV2 = vi[2]-1;
					
				m->v[vi[0]-1].norm = n[ni[0]-1];
				m->v[vi[1]-1].norm = n[ni[1]-1];
				m->v[vi[2]-1].norm = n[ni[2]-1];

				++curT;			
				
				m->t[curT].mat = 0;
			}
		}
		
		m->vc = curV;
		m->tc = curT;
	}
	/*
	{
		int i;
		for ( i = 0; i < m->vc; ++i )
		{
			m->v[i].pos.x *= 1.0f ;
			m->v[i].pos.y *= -1.0f;
			m->v[i].pos.z *= -1.0f;
			
			m->v[i].pos.z -= 10.0f;
			m->v[i].pos.y += 5.0f;
		}
	}
*/
	return 0;
}

void rt_mesh_add( rt_render_pipe *pRp, rt_mesh *m )
{
	rt_render_pipe_add_triangles( pRp, m->v, m->t, m->vc, m->tc );
}
