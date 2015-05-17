#include <cairo.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "rt.h"

#define SPHERE_COUNT           10
#define POINT_LIGHTS_COUNT     1
#define SQUARES_COUNT_SQRT     150
#define BOTTOM                 1

#define MATERIAL_COUNT         2 + SPHERE_COUNT

#define RENDER_TO_PNGS         0
#define RENDER_TO_SCREEN       1
#define PNGS_PATH              "pngs/"

#define WIDTH                  1280 //1920
#define HEIGHT                 768  //1080

typedef struct _wave
{
	float period;
	float amplitude;
	float angle;
	float speed;
} wave;

GtkWidget *mainWindow;
GtkWidget *drawArea;

cairo_surface_t *csur;
rt_argb *data;

guint rotTID;

rt_render_pipe renderPipe;

rt_sphere sp[SPHERE_COUNT];
rt_triangle tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 10];
rt_vertex vx[(SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) + 20];
rt_point_light lt[POINT_LIGHTS_COUNT];
rt_float koef[SPHERE_COUNT];
rt_material mt[MATERIAL_COUNT];

rt_plane pl;

rt_float t = 0.0f;
int w, h;

rt_float plSz = 150.0f;
rt_float plHt = 20.0f;
double avr = 0.0f;
int frm = 0;
unsigned long int pngN = 0;
float centralSpherePosY = 0.0f;
float dT = 0.0001f;
float oldDT = 0.001f;
int paused = 0;
rt_vector3 camPos;

//parameter for spheres moving equasions
int rotStep( gpointer data )
{
	if ( t > 2*M_PI )
		t = 0.0f;

	t += dT;

	gtk_widget_queue_draw(mainWindow);

	return 1;
}

void windowDestroy( GtkWidget *wgt, gpointer u )
{
	if ( RENDER_TO_PNGS )
	{
		int save = open( "save", O_TRUNC | O_CREAT | O_WRONLY, 
			S_IRWXU | S_IRWXG | S_IRWXO );

		write( save, (void *)(&t), sizeof(rt_float) );
		write( save, (void *)(&dT), sizeof(rt_float) );
		write( save, (void *)(&pngN), sizeof(unsigned long int) );

		close(save);
	}
	else
		printf( "t = %1.10f, dT = %1.10f\n", t, dT );

	g_source_remove( rotTID );
}

void keyPress( GtkWidget *w, GdkEvent *e, gpointer usrData )
{
	rt_camera *cam = NULL;
	rt_matrix4 m;
	rt_matrix4 tmp;

	rt_render_pipe_get_camera( &renderPipe, &cam );

	switch ( e->key.keyval )
	{
	case GDK_KEY_u:
		rt_matrix4_create_rotate( &m, M_PI*0.01f, RT_AXIS_Z );
	
		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_bracketleft:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f, RT_AXIS_Z );

		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_p:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f, RT_AXIS_Y );

		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_i:
		rt_matrix4_create_rotate( &m, M_PI*0.01f, RT_AXIS_Y );

		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_apostrophe:
		rt_matrix4_create_rotate( &m, M_PI*0.01f, RT_AXIS_X );

		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_j:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f,RT_AXIS_X );

		rt_matrix4_create_translate( &tmp, -camPos.x, 
			-camPos.y, -camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, camPos.x, 
			camPos.y, camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_semicolon:
		rt_matrix4_create_translate( &m, 0.0f, 1.0f, 0.0f );
		camPos.y -= 1.0f;
		goto applyTransform;

	case GDK_KEY_k:
		rt_matrix4_create_translate( &m, 0.0f, -1.0f, 0.0f );
		camPos.y += 1.0f;
		goto applyTransform;

	case GDK_KEY_o:
		rt_matrix4_create_translate( &m, 1.0f, 0.0f, 0.0f );
		camPos.x -= 1.0f;
		goto applyTransform;

	case GDK_KEY_l:
		rt_matrix4_create_translate( &m, -1.0f, 0.0f, 0.0f );
		camPos.x += 1.0f;
		goto applyTransform;

	case GDK_KEY_space:
		camPos.z += 1.0f;
		rt_matrix4_create_translate( &m, 0.0f, 0.0f, -1.0f );
		goto applyTransform;

	case GDK_KEY_comma:
		camPos.z -= 1.0f;
		rt_matrix4_create_translate( &m, 0.0f, 0.0f, 1.0f );
		goto applyTransform;
	
	case GDK_KEY_t:
		centralSpherePosY -= 0.5f;
		break;

	case GDK_KEY_y:
		centralSpherePosY += 0.5f;
		break;

	case GDK_KEY_g:
		if ( paused )
			dT = oldDT;
		else
		{
			oldDT = dT;
		 	dT = 0.0f;
		}
		paused = !paused;

		break;

	case GDK_KEY_equal:
		dT += 0.0001f;
		break;

	case GDK_KEY_minus:
		dT -= 0.0001f;
		break;

	default:
		break;

	applyTransform:
		rt_matrix4_mult( &(cam->world), &m,
			&(cam->world) );
		gtk_widget_queue_draw(mainWindow);
	}


	rt_render_pipe_free_camera( &renderPipe, cam );
}
void resizeImage()
{
	GtkAllocation alloc;
	rt_camera *cam = NULL;

	
	gtk_widget_get_allocation( drawArea, &alloc );

	w = alloc.width;
	h = alloc.height;

	csur = cairo_image_surface_create( CAIRO_FORMAT_RGB24, w, h );
	data = (rt_argb *) cairo_image_surface_get_data( csur );

	rt_render_pipe_get_camera( &renderPipe, &cam );
	rt_matrix4_create_projection( &(cam->viewToPersp), (float)w/(float)h,
		0.125*M_PI );

	rt_render_pipe_free_camera( &renderPipe, cam );

	rt_render_pipe_set_image_size( &renderPipe, w, h );
}

void buildPlaneOfTriangles()
{
	rt_vector3 v[4];
	rt_color col[5];
	wave wv[7];
	int wavesCount = 10;

	int i, j, k;
	int pointsInRow = SQUARES_COUNT_SQRT+1;

	rt_color_create( col, 0.5f, 0.223f, 0.345f, 0.474f );   //color
	rt_color_create( col+1, 0.0f, 0.05f, 0.05f, 0.05f );       //ambient
	rt_color_create( col+2, 0.0f, 0.5f, 0.5f, 0.5f );       //diffuse
	rt_color_create( col+3, 3.5f, 0.25f, 0.25f, 0.25f );    //specular
	rt_color_create( col+4, 0.0f, 0.3f, 0.3f, 0.3f );       //reflect
	rt_material_create( mt, col, col+1, col+2, 
		col+3, col+4, 0.05, 1.33f );
		
	#pragma GCC diagnostic ignored "-Wdiv-by-zero"
	rt_vector3 e1, e2;
	rt_float dt = plSz/SQUARES_COUNT_SQRT;
	rt_float xPos = -plSz*0.5f, zPos = 50.0f-plSz*0.5f;
	#pragma GCC diagnostic push

	wv[0].period = 100.0f; wv[0].amplitude = 1.5f;
	wv[0].angle = M_PI*0.5f; wv[0].speed = 500.0f;

	wv[1].period = 79.16677f; wv[1].amplitude = 0.25f;
	wv[1].angle = 2.9428474f; wv[1].speed = 256.9786f;

	wv[2].period = 144.04169f; wv[2].amplitude = 0.25f;
	wv[2].angle = -1.8729727f; wv[2].speed = 160.6270f;

	wv[3].period = 79.15634f; wv[3].amplitude = 0.25f;
	wv[3].angle = 1.8583484f; wv[3].speed = 239.9041f;

	wv[4].period = 121.51403f; wv[4].amplitude = 0.5f;
	wv[4].angle = -1.5343947f; wv[4].speed = 230.2120f;

	wv[5].period = 72.40281f; wv[5].amplitude = 0.5f;
	wv[5].angle = 1.4335813f; wv[5].speed = 300.9681f;

	wv[6].period = 81.57271f; wv[6].amplitude = 0.5f;
	wv[6].angle = -1.2057098f; wv[6].speed = 280.5781f;

	//create vertex
	for ( i = 0; i < pointsInRow; ++i )
		for ( j = 0; j < pointsInRow; ++j )
		{
			float u = ((float) i) / pointsInRow;
			float v = ((float) j) / pointsInRow;
	
			float y = plHt;

			for ( k = 0; k < wavesCount; ++k )
			{
				y += sin(wv[k].period 
					* (-cos(wv[k].angle)*u+sin(wv[k].angle)*v) 
					+ t*wv[k].speed) * wv[k].amplitude;
			}

			rt_vector3_create( 
				&(vx[i*pointsInRow+j].pos),
				xPos+i*dt, y,
				zPos+j*dt );

			rt_vector3_create( 
				&(vx[i*pointsInRow+j].norm),
				0.0f, -1.0f, 0.0f );
		}

	//compute indeces
	k = 0;
	for ( i = 0; i < SQUARES_COUNT_SQRT; ++i )
		for ( j = 0; j < SQUARES_COUNT_SQRT; ++j )
		{
			tr[k].pV0 = pointsInRow*i + j;
			tr[k].pV1 = pointsInRow*(i+1) + j;
			tr[k].pV2 = pointsInRow*i + (j+1);
			tr[k].mat = 0;

			++k;

			tr[k].pV0 = pointsInRow*(i+1) + (j+1);
			tr[k].pV1 = pointsInRow*i + (j+1);
			tr[k].pV2 = pointsInRow*(i+1) + j;
			tr[k].mat = 0;

			++k;
		}

	//compute normals
	for ( i = 0; i < SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2; ++i )
	{
		int i0, i1, i2;

		i0 = tr[i].pV0;
		i1 = tr[i].pV1;
		i2 = tr[i].pV2;

		rt_vector3_sub( &(vx[i0].pos), 
			&(vx[i1].pos), &e1 );
		rt_vector3_sub( &(vx[i0].pos), 
			&(vx[i2].pos), &e2 );
		rt_vector3_cross( &e1, &e2, v );
				
		rt_vector3_add( &(vx[i0].norm), 
			v, &(vx[i0].norm) );
		rt_vector3_add( &(vx[i1].norm), 
			v, &(vx[i1].norm) );
		rt_vector3_add( &(vx[i2].norm), 
			v, &(vx[i2].norm) );
	}
		
	//normalize normals	
	for ( i = 0; i < pointsInRow*pointsInRow; ++i )
		rt_vector3_normalize( &(vx[i].norm), 
			&(vx[i].norm) );

}

void draw( GtkWidget *wgt, cairo_t *cr, gpointer ud )
{
	struct timespec curTime;
	long long int c, p;
	int i, s;
	rt_float radius;
	rt_vector3 center;
	rt_argb *renderedImage;

	//set timer for fps counter
	clock_gettime( CLOCK_REALTIME, &curTime );
	p = curTime.tv_sec*1000000000LL + curTime.tv_nsec;
	s = curTime.tv_sec;

	if ( SQUARES_COUNT_SQRT )
		buildPlaneOfTriangles();

	for ( i = 0; i < MATERIAL_COUNT; ++i ) 
		rt_render_pipe_add_material( &renderPipe, mt+i, i );

	//add light to render pipe
	for ( i = 0; i < POINT_LIGHTS_COUNT; ++i )
	{
		rt_render_pipe_add_light( &renderPipe, (void *)(lt+i),
			RT_LT_POINT );
	}

	//add primitives to render pipe
	if ( SPHERE_COUNT > 0 )
	{
		sp[0].pos.y = centralSpherePosY;
		rt_render_pipe_add_primitive( &renderPipe, 
			(void *)(&sp[0]), RT_PT_SPHERE );
	}

	//make the other spheres flying around the first one,
	radius = 25.0f + 25.0f*(0.5f+sin(10.0f*t)*0.5f);
	center.x = 0.0f;
	center.y = 0.0f;
	center.z = 50.0f;

	for ( i = 1; i < SPHERE_COUNT; ++i )
	{
		sp[i].pos.x = center.x + sin( t + (rt_float)(i-1) * 2.0f
			 / (rt_float)(SPHERE_COUNT-1) * M_PI) * radius;
		sp[i].pos.y = center.y + koef[i]*0.5f*sin(koef[i]*0.5f*t);
		sp[i].pos.z = center.z + cos( t + (rt_float)(i-1) * 2.0f
			/ (rt_float)(SPHERE_COUNT-1) * M_PI) * radius;

		//add sphere to render pipe	
		rt_render_pipe_add_primitive( &renderPipe, (void *)(&sp[i]), 
			RT_PT_SPHERE );
	}

	//add triangles
	rt_render_pipe_add_triangles( &renderPipe, vx, tr,
		(SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) + (BOTTOM ? 20 : 0),
		SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + (BOTTOM ? 10 : 0) );

	//draw to buffer
	renderedImage = rt_render_pipe_draw( &renderPipe );
	memcpy( data, renderedImage, sizeof(rt_argb) * w * h );
	
	//draw buffer to screen
	if ( RENDER_TO_SCREEN )
	{
		cairo_set_source_surface(cr, csur, 0, 0 );
		cairo_paint(cr);
	}

	//draw to files
	if ( RENDER_TO_PNGS )
	{
		char tmp[255];
		sprintf( tmp, "%s%lu.png", PNGS_PATH, pngN++ );
		
		cairo_surface_write_to_png( csur, tmp );
	}

	//fps counter
	clock_gettime( CLOCK_REALTIME, &curTime );

	c = curTime.tv_sec*1000000000LL + curTime.tv_nsec - p;
	avr += 1000000000.0/(double)(c);
	++frm;

	if ( curTime.tv_sec != s )
	{
		printf( "fps %f\n", (double) avr / (double) frm );
		avr = 0.0;
		frm = 0;
	}
}

int main( int argc, char *argv[] )
{
	int i;	
	time_t initialTime = time(0);

	rt_camera frust;
	rt_vector3 v[4];
	rt_color col[5];

	srand(time(0));
	
	//create surface for render
	w = WIDTH;
	h = HEIGHT;

	csur = cairo_image_surface_create( CAIRO_FORMAT_RGB24, w, h );
	data = (rt_argb *) cairo_image_surface_get_data( csur );

	//init rt
	rt_init( *argv );

	//initiliaze render pipe
	rt_camera_create( &frust, (float)w / (float)h, 0.125*M_PI );

	rt_render_pipe_create( &renderPipe, w, h );

	rt_render_pipe_set_camera( &renderPipe, &frust );

	//create dir for writing images, if it doesn't exists
	if ( RENDER_TO_PNGS )
	{
		struct stat s = {0};
		int save;

		if ( stat( PNGS_PATH, &s ) == -1 )
			mkdir( PNGS_PATH, S_IRWXU | S_IRWXG | S_IRWXO );

		if ( (save = open( "save", O_RDONLY )) >= 0 )
		{
			read( save, (void *)(&t), sizeof(rt_float) );
			read( save, (void *)(&dT), sizeof(rt_float) );
			read( save, (void *)(&pngN), sizeof(unsigned long int) );
			
			t += dT;
		}

	}

	//initialize render pipe and create objects to draw
		
	//create fisrt sphere
	if ( SPHERE_COUNT > 0 )
	{
		rt_color_create( col, 0.5f, 0.0f, 0.5f, 1.0f );      //color
		rt_color_create( col+1, 0.0f, 0.05f, 0.05f, 0.05f );    //ambient
		rt_color_create( col+2, 0.0f, 0.2f, 0.2f, 0.2f );    //diffuse
		rt_color_create( col+3, 15.0f, 0.2f, 0.2f, 0.2f );   //specular
		rt_color_create( col+4, 0.0f, 0.5f, 0.5f, 0.5f );    //reflect
		rt_material_create( mt+1, col, col+1, col+2, 
			col+3, col+4, 0.0, 1.33f );
	
		rt_vector3_create( v, 0.0f, 0.0f, 50.0f );
		rt_sphere_create( sp, v, 10.0f, 1 );
	}
 		
	//create other spheres
	for ( i = 1; i < SPHERE_COUNT; ++i )
	{
		rt_color_create( col, 1.0f, 
			(rt_float)(rand()%16365)/16384.0f, 
			(rt_float)(rand()%16365)/16384.0f, 
			(rt_float)(rand()%16365)/16384.0f );
		rt_color_create( col+1, 0.0f, 0.05f, 0.05f, 0.05f ); //ambient
		rt_color_create( col+2, 0.0f, 0.5f, 0.5f, 0.5f );    //diffuse
		rt_color_create( col+3, 15.0f, 0.5f, 0.5f, 0.5f );   //specular
		rt_color_create( col+4, 0.0f, 0.5f, 0.5f, 0.5f );    //reflect
		rt_material_create( mt+1+i, col, col+1, col+2, 
			col+3, col+4, 0.0f, 1.0f );
			
		rt_sphere_create( &sp[i], v, 3.66541f, 1+i );

		koef[i] = (rt_float)(rand()%101);
	}

	//create point light
	rt_color_create( col, 1.0f, 1.0f, 1.0f, 1.0f );
	rt_vector3_create( v, -20.0f, -35.0f, 0.0f );

	for ( i = 0; i < POINT_LIGHTS_COUNT; ++i )
		rt_point_light_create( lt+i, v, 100.0f, col );

	{
		rt_camera *cam = NULL;
		rt_render_pipe_get_camera( &renderPipe, &cam );

		//set camera position
		cam->world._11 = 1.0f;          
		cam->world._12 = 0.0f;          
		cam->world._13 = 0.0f;          
		cam->world._14 = 0.0f;          
		                                   
		cam->world._21 = 0.0f;          
		cam->world._22 = 0.968582f;     
		cam->world._23 = -0.248690f;    
		cam->world._24 = 0.0f;          
                                                   
		cam->world._31 = 0.0f;          
		cam->world._32 = 0.248690f;     
		cam->world._33 = 0.968582f;     
		cam->world._34 = 0.0f;          
		                                   
		cam->world._41 = 0.0f;          
		cam->world._42 = -12.0f;        
		cam->world._43 = -43.0f;        
		cam->world._44 = 1.0f;          
                                                   
		camPos.x = 0.0f;           
		camPos.y = 12.0f;          
		camPos.z = 43.0f; 
	
		rt_render_pipe_free_camera( &renderPipe, cam );
	}

	{
		rt_ulong vertexOffset = SQUARES_COUNT_SQRT 
			? (SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) : 0;
		float bottomHeight = 25.0f;
		float topHeight = 3.0f;
	
		rt_color_create( col, 1.0f, 1.0f, 1.0f, 1.0f );     //color
		rt_color_create( col+1, 0.0f, 0.05f, 0.05f, 0.05f );   //ambient
		rt_color_create( col+2, 0.0f, 1.0f, 1.0f, 1.0f );   //diffuse
		rt_color_create( col+3, 5.0f, 1.0f, 1.0f, 1.0f );   //specular
		rt_color_create( col+4, 0.0f, 0.0f, 0.0f, 0.0f );   //reflect
		rt_material_create( mt + SPHERE_COUNT + 1, col, col+1, col+2, 
			col+3, col+4, 0.0, 1.0f );
	
		rt_vector3_create( &(vx[vertexOffset].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 1].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 2].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 3].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );

		rt_vector3_create( &(vx[vertexOffset].norm),
			0.0f, -1.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 1].norm),
			0.0f, -1.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 2].norm),
			0.0f, -1.0f, 0.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 3].norm),
			0.0f, -1.0f, 0.0f );

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2].pV0 = vertexOffset;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2].pV1 = vertexOffset + 1;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2].pV2 = vertexOffset + 2;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2].mat = SPHERE_COUNT + 1;

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 1].pV0 = vertexOffset + 3;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 1].pV1 = vertexOffset + 1;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 1].pV2 = vertexOffset + 2;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 1].mat = SPHERE_COUNT + 1;


		rt_vector3_create( &(vx[vertexOffset + 4].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 5].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 6].pos),
			-plSz*0.5f, plHt - topHeight, plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 7].pos),
			-plSz*0.5f, plHt - topHeight, -plSz*0.5f+50.0f );

		rt_vector3_create( &(vx[vertexOffset + 4].norm),
			1.0f, 0.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 5].norm),
			1.0f, 0.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 6].norm),
			1.0f, 0.0f, 0.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 7].norm),
			1.0f, 0.0f, 0.0f );

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 2].pV0 = vertexOffset + 4;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 2].pV1 = vertexOffset + 5;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 2].pV2 = vertexOffset + 6;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 2].mat = SPHERE_COUNT + 1;

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 3].pV0 = vertexOffset + 7;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 3].pV1 = vertexOffset + 6;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 3].pV2 = vertexOffset + 5;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 3].mat = SPHERE_COUNT + 1;


		rt_vector3_create( &(vx[vertexOffset + 8].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 9].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 10].pos),
			plSz*0.5f, plHt - topHeight, plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 11].pos),
			plSz*0.5f, plHt - topHeight, -plSz*0.5f+50.0f );

		rt_vector3_create( &(vx[vertexOffset + 8].norm),
			-1.0f, 0.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 9].norm),
			-1.0f, 0.0f, 0.0f );
		rt_vector3_create( &(vx[vertexOffset + 10].norm),
			-1.0f, 0.0f, 0.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 11].norm),
			-1.0f, 0.0f, 0.0f );

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 4].pV0 = vertexOffset + 8;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 4].pV1 = vertexOffset + 9;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 4].pV2 = vertexOffset + 10;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 4].mat = SPHERE_COUNT + 1;

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 5].pV0 = vertexOffset + 11;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 5].pV1 = vertexOffset + 10;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 5].pV2 = vertexOffset + 9;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 5].mat = SPHERE_COUNT + 1;


		rt_vector3_create( &(vx[vertexOffset + 12].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 13].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 14].pos),
			-plSz*0.5f, plHt - topHeight, plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 15].pos),
			plSz*0.5f, plHt - topHeight, plSz*0.5f+50.0f );

		rt_vector3_create( &(vx[vertexOffset + 12].norm),
			0.0f, 0.0f, -1.0f );
		rt_vector3_create( &(vx[vertexOffset + 13].norm),
			0.0f, 0.0f, -1.0f );
		rt_vector3_create( &(vx[vertexOffset + 14].norm),
			0.0f, 0.0f, -1.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 15].norm),
			0.0f, 0.0f, -1.0f );

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 6].pV0 = vertexOffset + 12;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 6].pV1 = vertexOffset + 13;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 6].pV2 = vertexOffset + 14;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 6].mat = SPHERE_COUNT + 1;

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 7].pV0 = vertexOffset + 15;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 7].pV1 = vertexOffset + 14;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 7].pV2 = vertexOffset + 13;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 7].mat = SPHERE_COUNT + 1;


		rt_vector3_create( &(vx[vertexOffset + 16].pos),
			-plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 17].pos),
			plSz*0.5f, plHt - topHeight + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 18].pos),
			-plSz*0.5f, plHt - topHeight, -plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 19].pos),
			plSz*0.5f, plHt - topHeight, -plSz*0.5f+50.0f );

		rt_vector3_create( &(vx[vertexOffset + 16].norm),
			0.0f, 0.0f, 1.0f );
		rt_vector3_create( &(vx[vertexOffset + 17].norm),
			0.0f, 0.0f, 1.0f );
		rt_vector3_create( &(vx[vertexOffset + 18].norm),
			0.0f, 0.0f, 1.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 19].norm),
			0.0f, 0.0f, 1.0f );

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 8].pV0 = vertexOffset + 16;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 8].pV1 = vertexOffset + 17;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 8].pV2 = vertexOffset + 18;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 8].mat = SPHERE_COUNT + 1;

		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 9].pV0 = vertexOffset + 19;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 9].pV1 = vertexOffset + 18;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 9].pV2 = vertexOffset + 17;
		tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 9].mat = SPHERE_COUNT + 1;

	}

	//set timeout for render
	rotTID = g_timeout_add( (guint)1, rotStep, NULL );

	//create main window
	gtk_init( &argc, &argv );

	
	mainWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL );

	drawArea = gtk_drawing_area_new();

	gtk_widget_set_hexpand( drawArea, 1 );
	gtk_widget_set_vexpand( drawArea, 1 );

	gtk_container_add( GTK_CONTAINER(mainWindow), drawArea );

	g_signal_connect( mainWindow, "destroy", G_CALLBACK(gtk_main_quit), 
		NULL );
	g_signal_connect( G_OBJECT(drawArea), "draw", G_CALLBACK(draw), NULL );
	g_signal_connect( G_OBJECT(drawArea), "size_allocate", 
		G_CALLBACK(resizeImage), NULL );
	g_signal_connect( mainWindow, "key-press-event", 
		G_CALLBACK(keyPress), NULL );
	g_signal_connect( mainWindow, "destroy",
		G_CALLBACK(windowDestroy), NULL );
	
	gtk_window_resize( GTK_WINDOW(mainWindow), w, h );
	gtk_widget_show_all(mainWindow);

	gtk_main();

	initialTime = time(0) - initialTime;
	printf( "total rendering time: %ldh, %ldm, %lds\n", 
		initialTime / 3600,
		(initialTime % 3600) / 60,
		(initialTime % 60) );

	rt_render_pipe_cleanup( &renderPipe );

	return 0;
}
