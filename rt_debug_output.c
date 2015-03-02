#include "rt_debug_output.h"

void rt_dout_matrix4( rt_matrix4 *pM )
{
	printf( "%f, %f, %f, %f\n",
		pM->_11, pM->_12, pM->_13, pM->_14 );
	printf( "%f, %f, %f, %f\n",
		pM->_21, pM->_22, pM->_23, pM->_24 );
	printf( "%f, %f, %f, %f\n",
		pM->_31, pM->_32, pM->_33, pM->_34 );
	printf( "%f, %f, %f, %f",
		pM->_41, pM->_42, pM->_43, pM->_44 );
	
}

void rt_dout_vector3( rt_vector3 *pV )
{
	printf( "%f, %f, %f", pV->x, pV->y, pV->z );	
}

void tabulation( int count )
{
	int i;

	for ( i = 0; i < count; ++i )
		printf( "\t" );
}


void rt_dout_kdtree( rt_verticle *pV, rt_triangle *pTr, 
	rt_render_pipe *pRp, rt_kdtree_node *pNode, int depth )
{
	tabulation( depth );
	printf( "{\n" );

	if ( pNode->isLast )
	{
		int i;
		tabulation( depth + 1 );
		printf( "prims count: %lu", pNode->primsCount );;
		printf( "\n" );

		for ( i = 0; i < pNode->primsCount; ++i )
		{
			tabulation( depth + 1 );
			printf( "%luth triangle:", pNode->prims[i] );
			printf( "\n" );
			tabulation( depth + 1 );
			printf( "{\n" );
			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pNode->prims[i]].pV0].pos) );
			printf( "\n" );

			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pNode->prims[i]].pV1].pos) );
			printf( "\n" );

			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pNode->prims[i]].pV2].pos) );
			printf( "\n" );



			tabulation( depth + 1 );
			printf( "}\n" );
		}
	}
	else
	{
		tabulation( depth + 1 );
		printf( "separation axis: " );	
		switch ( pNode->axis )
		{
		case RT_AXIS_X:
			printf( "X" );
			break;

		case RT_AXIS_Y:
			printf( "Y" );
			break;

		case RT_AXIS_Z:
			printf( "Z" );
			break;
		}
		printf( "\n" );
		tabulation( depth + 1 );
		printf( "separation value: %f\n", pNode->sep );
		tabulation( depth + 1 );
		printf( "left node:\n" );
		rt_dout_kdtree( pV, pTr, pRp, pNode->leftNode, depth + 1 );
		tabulation( depth + 1 );
		printf( "right node:\n" );
		rt_dout_kdtree( pV, pTr, pRp, pNode->rightNode, depth + 1 );
	}

	tabulation( depth );
	printf( "}\n\n" );
}

void rt_cl_dout_kdtree( rt_verticle *pV, rt_triangle *pTr, rt_cl_kdtree_node *pN, 
	rt_ulong *pP, rt_render_pipe *pRp, rt_ulong nodeN, int depth )
{
	tabulation( depth );
	printf( "node %lu {\n", nodeN );
	
	if ( pN[nodeN].isLast )
	{
		int i;
		tabulation( depth + 1 );
		printf( "prims count: %lu", pN[nodeN].primsCount );;
		printf( "\n" );

		for ( i = 0; i < pN[nodeN].primsCount; ++i )
		{
			tabulation( depth + 1 );
			printf( "%luth triangle:", pP[pN[nodeN].prims + i] );
			printf( "\n" );
			tabulation( depth + 1 );
			printf( "{\n" );
			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pP[pN[nodeN].prims + i]].pV0].pos) );
			printf( "\n" );

			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pP[pN[nodeN].prims + i]].pV1].pos) );
			printf( "\n" );

			tabulation( depth + 2 );
			rt_dout_vector3( &(pV[pTr[pP[pN[nodeN].prims + i]].pV2].pos) );
			printf( "\n" );



			tabulation( depth + 1 );
			printf( "}\n" );
		}
	}
	else
	{
		tabulation( depth + 1 );
		printf( "separation axis: " );	
		switch ( pN[nodeN].axis )
		{
		case RT_AXIS_X:
			printf( "X" );
			break;

		case RT_AXIS_Y:
			printf( "Y" );
			break;

		case RT_AXIS_Z:
			printf( "Z" );
			break;
		}
		printf( "\n" );
		tabulation( depth + 1 );
		printf( "separation value: %f\n", pN[nodeN].sep );
		tabulation( depth + 1 );
		printf( "left node:\n" );
		rt_cl_dout_kdtree( pV, pTr, pN, pP, pRp, pN[nodeN].leftNode, depth + 1 );
		tabulation( depth + 1 );
		printf( "right node:\n" );
		rt_cl_dout_kdtree( pV, pTr, pN, pP, pRp, pN[nodeN].rightNode, depth + 1 );
	}

	tabulation( depth );
	printf( "}\n\n" );
}
