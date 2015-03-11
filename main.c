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

#include "rt_types.h"
#include "rt_funcs_render_pipe.h"

#define SPHERE_COUNT           10
#define POINT_LIGHTS_COUNT     1
#define SQUARES_COUNT_SQRT     50
#define BOTTOM                 0

#define MATERIAL_COUNT         2 + SPHERE_COUNT

#define RENDER_TO_PNGS         0
#define PNGS_PATH              "pngs/"

#define WIDTH                  1280
#define HEIGHT                 640

typedef struct _wave
{
	float period;
	float amplitude;
	rt_vector3 dir;
} wave;

GtkWidget *mainWindow;
GtkWidget *drawArea;

cairo_surface_t *csur;
rt_argb *data;

guint rotTID;

rt_render_pipe renderPipe;

rt_sphere sp[SPHERE_COUNT];
rt_triangle tr[SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + 2];
rt_verticle vx[(SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) + 4];
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
	
		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_bracketleft:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f, RT_AXIS_Z );

		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_p:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f, RT_AXIS_Y );

		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_i:
		rt_matrix4_create_rotate( &m, M_PI*0.01f, RT_AXIS_Y );

		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_apostrophe:
		rt_matrix4_create_rotate( &m, M_PI*0.01f, RT_AXIS_X );

		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_j:
		rt_matrix4_create_rotate( &m, -M_PI*0.01f,RT_AXIS_X );

		rt_matrix4_create_translate( &tmp, -cam->camPos.x, 
			-cam->camPos.y, -cam->camPos.z );
		rt_matrix4_mult( &m, &tmp, &m );

		rt_matrix4_create_translate( &tmp, cam->camPos.x, 
			cam->camPos.y, cam->camPos.z );

		rt_matrix4_mult( &tmp, &m, &m );


		goto applyTransform;

	case GDK_KEY_semicolon:
		rt_matrix4_create_translate( &m, 0.0f, 1.0f, 0.0f );
		cam->camPos.y -= 1.0f;
		goto applyTransform;

	case GDK_KEY_k:
		rt_matrix4_create_translate( &m, 0.0f, -1.0f, 0.0f );
		cam->camPos.y += 1.0f;
		goto applyTransform;

	case GDK_KEY_o:
		rt_matrix4_create_translate( &m, 1.0f, 0.0f, 0.0f );
		cam->camPos.x -= 1.0f;
		goto applyTransform;

	case GDK_KEY_l:
		rt_matrix4_create_translate( &m, -1.0f, 0.0f, 0.0f );
		cam->camPos.x += 1.0f;
		goto applyTransform;

	case GDK_KEY_space:
		cam->camPos.z += 1.0f;
		rt_matrix4_create_translate( &m, 0.0f, 0.0f, -1.0f );
		goto applyTransform;

	case GDK_KEY_comma:
		cam->camPos.z -= 1.0f;
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
		rt_matrix4_inverse(  &(cam->world),  &(cam->worldInverse) );
		rt_matrix4_transpose( &(cam->worldInverse),
			&(cam->worldInvTr) );

		gtk_widget_queue_draw(mainWindow);
	}

}
void resizeImage()
{
	GtkAllocation alloc;
	
	gtk_widget_get_allocation( drawArea, &alloc );

	w = alloc.width;
	h = alloc.height;
}

void buildPlaneOfTriangles()
{
	rt_vector3 v[4];

	rt_color col[5];

	int i, j, k;

	rt_color_create( col, 0.5f, 0.223f, 0.345f, 0.474f );   //color
	rt_color_create( col+1, 0.0f, 1.0f, 1.0f, 1.0f );       //ambient
	rt_color_create( col+2, 0.0f, 0.75f, 0.75f, 0.75f );    //diffuse
	rt_color_create( col+3, 3.5f, 0.25f, 0.25f, 0.25f );    //specular
	rt_color_create( col+4, 0.0f, 0.3f, 0.3f, 0.3f );       //reflect
	rt_material_create( mt, col, col+1, col+2, 
		col+3, col+4, 0.005, 1.33f );
		
	#pragma GCC diagnostic ignored "-Wdiv-by-zero"
	rt_vector3 e1, e2;
	rt_float dt = plSz/SQUARES_COUNT_SQRT;
	rt_float xPos = -plSz*0.5f, zPos = 50.0f-plSz*0.5f;
	int pointsInRow = SQUARES_COUNT_SQRT+1;
	#pragma GCC diagnostic push


	//create vertex
	for ( i = 0; i < pointsInRow; ++i )
		for ( j = 0; j < pointsInRow; ++j )
		{
			float u = ((float) i) / pointsInRow;
			float v = ((float) j) / pointsInRow;
			float y = plHt
				+ sin(100.0 * u + t*0.0f) * 0.5f
				+ cos(25.0 * v + t*125.0f) * 2.0f
				+ cos(50.0 * v + t*250.0f) * 1.0f
				+ cos(200.0 * v + t*500.0f) * 0.5f;

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
		(SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) + (BOTTOM ? 4 : 0),
		SQUARES_COUNT_SQRT*SQUARES_COUNT_SQRT*2 + (BOTTOM ? 2 : 0) );

	//draw to buffer
	renderedImage = rt_render_pipe_draw( &renderPipe );

	//draw buffer to screen
	memcpy( data, renderedImage, sizeof(rt_argb) * WIDTH * HEIGHT );
	cairo_set_source_surface(cr, csur, 0, 0 );
	cairo_paint(cr);

	//render to files
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
	rt_camera frust;
	rt_vector3 v[4];
	rt_color col[5];

	srand(time(0));
	
	//create surface for render
	csur = cairo_image_surface_create( CAIRO_FORMAT_RGB24, WIDTH, HEIGHT );
	data = (rt_argb *) cairo_image_surface_get_data( csur );
	w = WIDTH;
	h = HEIGHT;

	//init rt
	rt_init( *argv );

	//create dir for writing images, if it doesn't exists
	if ( RENDER_TO_PNGS )
	{
		struct stat s = {0};

		if ( stat( PNGS_PATH, &s ) == -1 )
			mkdir( PNGS_PATH, S_IRWXU | S_IRWXG | S_IRWXO );

	}

	//initialize render pipe and create objects to draw
		
	//create fisrt sphere
	if ( SPHERE_COUNT > 0 )
	{
		rt_color_create( col, 0.5f, 0.0f, 0.5f, 1.0f );      //color
		rt_color_create( col+1, 0.0f, 0.0f, 0.0f, 0.0f );    //ambient
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
		rt_color_create( col+1, 0.0f, 0.0f, 0.0f, 0.0f );    //ambient
		rt_color_create( col+2, 0.0f, 0.5f, 0.5f, 0.5f );    //diffuse
		rt_color_create( col+3, 15.0f, 1.0f, 1.0f, 1.0f );   //specular
		rt_color_create( col+4, 0.0f, 0.5f, 0.5f, 0.5f );    //reflect
		rt_material_create( mt+1+i, col, col+1, col+2, 
			col+3, col+4, 0.0f, 1.0f );
			
		rt_sphere_create( &sp[i], v, 3.66541f, 1+i );

		koef[i] = (rt_float)(rand()%101);
	}

	//create point light
	rt_color_create( col, 1.0f, 1.0f, 1.0f, 1.0f );
	rt_vector3_create( v, -20.0f, -35.0f, 0.0f );

	rt_point_light_create( lt, v, 100.0f, col );

	//initiliaze render pipe
	rt_camera_create( &frust, WIDTH, HEIGHT, 0.125*M_PI, 100.0f );

	rt_render_pipe_create( &renderPipe, WIDTH, HEIGHT, data );

	rt_render_pipe_set_camera( &renderPipe, &frust );

	//set camera position
	renderPipe.cam->world._11 = 1.0f;          
	renderPipe.cam->world._12 = 0.0f;          
	renderPipe.cam->world._13 = 0.0f;          
	renderPipe.cam->world._14 = 0.0f;          
		                                   
	renderPipe.cam->world._21 = 0.0f;          
	renderPipe.cam->world._22 = 0.968582f;     
	renderPipe.cam->world._23 = -0.248690f;    
	renderPipe.cam->world._24 = 0.0f;          
                                                   
	renderPipe.cam->world._31 = 0.0f;          
	renderPipe.cam->world._32 = 0.248690f;     
	renderPipe.cam->world._33 = 0.968582f;     
	renderPipe.cam->world._34 = 0.0f;          
		                                   
	renderPipe.cam->world._41 = 0.0f;          
	renderPipe.cam->world._42 = -18.0f;        
	renderPipe.cam->world._43 = -43.0f;        
	renderPipe.cam->world._44 = 1.0f;          
                                                   
	renderPipe.cam->camPos.x = 0.0f;           
	renderPipe.cam->camPos.y = 18.0f;          
	renderPipe.cam->camPos.z = 43.0f;          

	{
		rt_ulong vertexOffset = SQUARES_COUNT_SQRT ? (SQUARES_COUNT_SQRT+1)*(SQUARES_COUNT_SQRT+1) : 0;
		float bottomHeight = 25.0f;
	
		rt_color_create( col, 1.0f, 1.0f, 1.0f, 1.0f );     //color
		rt_color_create( col+1, 0.0f, 0.0f, 0.0f, 0.0f );   //ambient
		rt_color_create( col+2, 0.0f, 1.0f, 1.0f, 1.0f );   //diffuse
		rt_color_create( col+3, 5.0f, 1.0f, 1.0f, 1.0f );   //specular
		rt_color_create( col+4, 0.0f, 0.0f, 0.0f, 0.0f );   //reflect
		rt_material_create( mt + SPHERE_COUNT + 1, col, col+1, col+2, 
			col+3, col+4, 0.0, 1.0f );
	
		rt_vector3_create( &(vx[vertexOffset].pos),
			plSz*0.5f, plHt + bottomHeight, plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 1].pos),
			plSz*0.5f, plHt + bottomHeight, -plSz*0.5f+50.0f );
		rt_vector3_create( &(vx[vertexOffset + 2].pos),
			-plSz*0.5f, plHt + bottomHeight, plSz*0.5f+50.0f ); 
		rt_vector3_create( &(vx[vertexOffset + 3].pos),
			-plSz*0.5f, plHt + bottomHeight, -plSz*0.5f+50.0f );
	
	
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

	gtk_widget_show_all(mainWindow);
	gtk_window_resize( GTK_WINDOW(mainWindow), WIDTH, HEIGHT );
	gtk_main();

	rt_cleanup( &renderPipe );

	return 0;
}
