#include "rt_meshes.h"
#include <fcntl.h>

char **rt_mesh_load_mtl( rt_mesh *m, char *path )
{
	FILE *file;
	char buf[255];
	rt_ulong curM;
	char **mtlTable = NULL;
	int l = strlen( path );
		
	if ( l != 0 && path[l - 1] == '\n' )
		path[l - 1] = '\0';

	m->mc = 0;
	if ( (file = fopen( path, "r" )) == NULL )
		return NULL;

	while ( fgets( buf, 255, file ) != NULL )
		if (  strncmp( buf, "newmtl", 6 ) == 0 )
			++(m->mc);

	m->mat = malloc( m->mc * sizeof(rt_material) );
	mtlTable = malloc( m->mc * sizeof(char *) );

	rewind( file );

	curM = -1;
	while ( fgets( buf, 255, file ) != NULL )
	{
		if ( strncmp( buf, "newmtl", 6 ) == 0 )
		{
			int l = strlen( buf );
			++curM;

			if ( buf[l - 1] == '\n' && l != 0 )
				buf[l - 1] = '\0';	
			
			mtlTable[curM] = malloc( (strlen( buf + 7 ) + 1) * sizeof(char) );
			strcpy( mtlTable[curM], buf + 7 );
		}
		else if ( strncmp( buf, "Ns", 2 ) == 0 )
			sscanf( buf + 3, "%f", &(m->mat[curM].specular.a) );
		else if ( strncmp( buf, "Ka", 2 ) == 0 )
		{
			sscanf( buf + 3, "%f %f %f", &(m->mat[curM].ambient.r),
				&(m->mat[curM].ambient.g),
				&(m->mat[curM].ambient.b) );
			m->mat[curM].ambient.a = 0.0f;
		}
		else if ( strncmp( buf, "Kd", 2 ) == 0 )
		{
			sscanf( buf + 3, "%f %f %f", &(m->mat[curM].diffuse.r),
				&(m->mat[curM].diffuse.g),
				&(m->mat[curM].diffuse.b) );
			m->mat[curM].diffuse.a = 0.0f;
		}
		else if ( strncmp( buf, "Ks", 2 ) == 0 )
		{
			sscanf( buf + 3, "%f %f %f", &(m->mat[curM].specular.r),
				&(m->mat[curM].specular.g),
				&(m->mat[curM].specular.b) );
		}
		else if ( strncmp( buf, "Ni", 2 ) == 0 )
			sscanf( buf + 3, "%f", &(m->mat[curM].optDens) );
		else if ( strncmp( buf, "d", 1 ) == 0 ) {}
			sscanf( buf + 2, "%f", &(m->mat[curM].color.a) );

	}


	fclose(file);
	
	return mtlTable;
}

void rt_restore_normals( rt_mesh *m )
{
	int i0, i1, i2;
	int i;
	rt_vector3 e1, e2, v;

	for ( i = 0; i < m->tc; ++i )
	{
		i0 = m->t[i].pV0;
		i1 = m->t[i].pV1;
		i2 = m->t[i].pV2;
	
		rt_vector3_sub( &(m->v[i0].pos), 
			&(m->v[i1].pos), &e1 );
		rt_vector3_sub( &(m->v[i0].pos), 
			&(m->v[i2].pos), &e2 );
		rt_vector3_cross( &e1, &e2, &v );
				
		rt_vector3_add( &(m->v[i0].norm), 
			&v, &(m->v[i0].norm) );
		rt_vector3_add( &(m->v[i1].norm), 
			&v, &(m->v[i1].norm) );
		rt_vector3_add( &(m->v[i2].norm), 
			&v, &(m->v[i2].norm) );
	}
		
	//normalize normals	
	for ( i = 0; i < m->vc; ++i )
		rt_vector3_normalize( &(m->v[i].norm), 
			&(m->v[i].norm) );
}

rt_ulong rt_mesh_get_triangles_in_face( char *f )
{
	int c = 0;
	int w = 1;

	while ( f[c] != '\0' )
	{
		if ( f[c] == ' ' || f[c] == '\t' )
		{
			++w;
			while ( f[c] == ' ' || f[c] == '\t' )
				++c;
		}
	
		++c;
	}

	if ( w == 3 )
		return 1;
	else if ( w == 4 )
		return 2;
	else
		return -1;
	
	return 1;
}

int rt_mesh_load_from_obj( rt_mesh *m, char *path )
{
	FILE *file = fopen( path, "r" );
	char buf[255];
	int i;
	rt_ulong curG = 0, curV = 0, curN = 0, curT = 0;
	rt_ulong nc = 0;
	rt_vector3 *n;
	char **mtlTable = NULL;

//	m->v = m->t = m->g = NULL;

	if ( m == NULL || path == NULL )
		exit( -1 );

	if ( file == NULL )
		return -1;
	
	m->gc = 0;
	m->vc = 0;
	m->tc = 0;
		
	while ( fgets( buf, 255, file ) != NULL )
	{
		if ( strncmp( buf, "o", 1 ) == 0 )
			++(m->gc);
		else if ( strncmp( buf, "vn", 2 ) == 0 )
			++nc;
		else if ( strncmp( buf, "vt", 2 ) == 0 ) {}
		else if ( strncmp( buf, "v", 1 ) == 0 )
			++(m->vc);
		else if ( strncmp( buf, "f", 1 ) == 0 )
			m->tc += rt_mesh_get_triangles_in_face( buf + 2 );
		else if ( strncmp( buf, "mtllib", 6 ) == 0 )
			mtlTable = rt_mesh_load_mtl( m, buf + 7 );
	}

	m->v = malloc( m->vc * sizeof(rt_vertex) );
	m->t = malloc( m->tc * sizeof(rt_triangle) );
	m->g = malloc( m->gc * sizeof(rt_mesh_group) );
	n = (nc != 0) ? malloc( nc * sizeof(rt_vector3) ) : NULL;

	rewind( file );
	
	while ( fgets( buf, 255, file ) != NULL )
	{
		if ( strncmp( buf, "o", 1 ) == 0 )
		{
			m->g[curG].voffset = curV;
			m->g[curG].toffset = curT;

			if ( curG > 0 )
			{
				m->g[curG - 1].vc = curV - m->g[curG - 1].voffset;
				m->g[curG - 1].tc = curT - m->g[curG - 1].toffset;
			}

			++curG;
		}
		
		else if ( strncmp( buf, "vn", 2 ) == 0 )
		{
			sscanf( buf + 2, "%f %f %f",
				&(n[curN].x),
				&(n[curN].y),
				&(n[curN].z) );

			rt_vector3_normalize( n+curN, n+curN );
			++curN;
		}
		else if ( strncmp( buf, "vt", 2 ) == 0 )
		{

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
			int l = strlen( buf );
			if ( l != 0 && buf[l - 1] == '\n' )
				buf[l - 1] = '\0';	

			if ( rt_mesh_get_triangles_in_face( buf + 2 ) == 1 )
			{
				int vi[3];
				int ni[3];
				int uvi[3];

				sscanf( buf + 2, "%d/%d/%d %d/%d/%d %d/%d/%d",
					vi, uvi, ni,
					vi + 1, uvi + 1, ni + 1,
					vi + 2, uvi + 2, ni + 2 );
				
				m->v[vi[0]-1].norm = n[ni[0]-1];
				m->v[vi[1]-1].norm = n[ni[1]-1];
				m->v[vi[2]-1].norm = n[ni[2]-1];
	
				m->t[curT].pV0 = vi[0]-1;
				m->t[curT].pV1 = vi[1]-1;
				m->t[curT].pV2 = vi[2]-1;
				
				++curT;

			}
			else if ( rt_mesh_get_triangles_in_face( buf + 2 ) == 2 )
			{
				int vi[4];
				int ni[4];
				int uvi[4];

				sscanf( buf + 2, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
					vi, uvi, ni,
					vi + 1, uvi + 1, ni + 1,
					vi + 2, uvi + 2, ni + 2,
					vi + 3, uvi + 2, ni + 3 );
					
				m->v[vi[0]-1].norm = n[ni[0]-1];
				m->v[vi[1]-1].norm = n[ni[1]-1];
				m->v[vi[2]-1].norm = n[ni[2]-1];
				m->v[vi[3]-1].norm = n[ni[3]-1];

				m->t[curT].pV0 = vi[0]-1;
				m->t[curT].pV1 = vi[1]-1;
				m->t[curT].pV2 = vi[2]-1;

				++curT;
				m->t[curT].pV1 = vi[0]-1;
				m->t[curT].pV0 = vi[3]-1;
				m->t[curT].pV2 = vi[2]-1;

				++curT;

			}

		}
		else if ( strncmp( buf, "usemtl", 6 ) == 0 )
		{
			int l = strlen( buf );
			if ( l != 0 && buf[l - 1] == '\n' )
				buf[l - 1] = '\0';	
		
			for ( i = 0; i < m->mc; ++i )
				if ( strcmp( buf + 7, mtlTable[i] ) == 0 )
					(m->g)[curG-1].mat = m->mat + i;
		}
	}
	
	m->g[curG - 1].vc = curV - m->g[curG - 1].voffset;
	m->g[curG - 1].tc = curT - m->g[curG - 1].toffset;


///////////////////////////////////////////////////////////////////////////////
// for model in flutter.obj and it's reversed by bagged blenger polygons     //
///////////////////////////////////////////////////////////////////////////////
/*
	for ( i = 0; i < m->g[1].vc; ++i )
	{
		rt_vector3_scalar_mult( &((m->v)[m->g[1].voffset + i].norm),
			-1.0f, &((m->v)[m->g[1].voffset + i].norm) );
	}

	for ( i = 0; i < m->g[0].vc; ++i )
	{
		rt_vector3_scalar_mult( &((m->v)[m->g[0].voffset + i].norm),
			-1.0f, &((m->v)[m->g[0].voffset + i].norm) );
	}
*/
///////////////////////////////////////////////////////////////////////////////


	for ( i = 0; i < m->mc; ++i )
	{
		m->mat[i].color.r = m->mat[i].color.g = m->mat[i].color.b = 1.0f;
		m->mat[i].reflect.r = m->mat[i].reflect.g = m->mat[i].reflect.b = 0.0f;

	}

	if ( n != NULL )
	{
		free( n );
		n = NULL;
	}
	else
		rt_restore_normals( m );

	if ( mtlTable != NULL )
	{
		for ( i = 0; i < m->mc; ++i )
		{
			free( mtlTable[i] );
			mtlTable[i] = NULL;
		}
		free( mtlTable );
		mtlTable = NULL;
	}

	return 0;
}



void rt_mesh_add( rt_render_pipe *pRp, rt_mesh *m, rt_ulong tg )
{
	if ( pRp == NULL || m == NULL )
		exit( -1 );

	rt_render_pipe_add_triangles( pRp, m->v, m->t, 0, 0, m->vc, m->tc, tg );
}

void rt_mesh_add_group( rt_render_pipe *pRp, rt_mesh *m, rt_ulong tg,
	rt_ulong g )
{
	if ( pRp == NULL || m == NULL )
		exit( -1 );
/*
	int i;
	for ( i = 0; i < m->gc; ++i )
	{
		printf( "%d\n", i );
		printf( "%f %f %f %f\n", m->g[i].mat->diffuse.r, m->g[i].mat->diffuse.g, 
			m->g[i].mat->diffuse.b, m->g[i].mat->diffuse.a );
		printf( "%f %f %f %f\n", m->g[i].mat->ambient.r, m->g[i].mat->ambient.g, 
			m->g[i].mat->ambient.b, m->g[i].mat->ambient.a );
		printf( "%f %f %f %f\n", m->g[i].mat->specular.r, m->g[i].mat->specular.g, 
			m->g[i].mat->specular.b, m->g[i].mat->specular.a );

	}


	printf( "***%lu %lu | %lu %lu\n", m->g[g].voffset, m->g[g].vc,
		m->g[g].toffset, m->g[g].tc );
*/
	rt_render_pipe_add_material( pRp, m->g[g].mat, tg );
	rt_render_pipe_add_triangles( pRp, m->v, m->t, 
		m->g[g].voffset, m->g[g].toffset, m->g[g].vc, m->g[g].tc, tg );
}

void rt_mesh_reserve( rt_mesh *m, rt_ulong vc, rt_ulong tc , rt_ulong gc )
{
	if ( m == NULL )
		exit( -1 );

	m->vc = vc;
	m->tc = tc;
	m->gc = gc;

	m->v = malloc( vc * sizeof(rt_vertex) );
	m->t = malloc( tc * sizeof(rt_vertex) );
	m->g = malloc( gc * sizeof(rt_vertex) );
}

void rt_mesh_release( rt_mesh *m )
{
	if ( m == NULL )
		exit( -1 );

	if ( m->v != NULL )
	{
		free( m->v );
		m->v = NULL;
	}
	
	if ( m->t != NULL )
	{
		free( m->t );
		m->t = NULL;
	}

	if ( m->g != NULL )
	{
		free( m->g );
		m->g = NULL;
	}
}
