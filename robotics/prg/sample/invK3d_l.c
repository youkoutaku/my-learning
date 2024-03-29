// ***********************************
// *** invK3d_l.c                  ***
// *** æ[ÊuÆRrsñÌÝè  ***
// *** (3-D Simulation)            ***
// *** CopyRight Shugen MA 98/5/16 ***
// *** modified by mori-zen        ***
// ***********************************

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "invK3d.h"
#include "glshapes.h"

// *** æ[Êu ***
void endPosition( len, tht, pos )
	double *len;  //  Link length : L0, L1, L2
	double *tht;  //  Joint theta :     J1, J2
	double *pos;  // End position :  X,  Y   
{
	pos[X] = len[L1]*cos(tht[J1]) + len[L2]*cos(tht[J1]+tht[J2]);
	pos[Y] = len[L1]*sin(tht[J1]) + len[L2]*sin(tht[J1]+tht[J2]);
}

// *** Rrsñ ***
void jacobianMatrix( len, tht, Jaco )
	double *len;
	double *tht;
	MAT *Jaco;
{
	MATelem(*Jaco,1,1)= -len[L1]*sin(tht[J1]) - len[L2]*sin(tht[J1]+tht[J2]);
	MATelem(*Jaco,1,2)= -len[L2]*sin(tht[J1]+tht[J2]);
	MATelem(*Jaco,2,1)=  len[L1]*cos(tht[J1]) + len[L2]*cos(tht[J1]+tht[J2]);
	MATelem(*Jaco,2,2)=  len[L2]*cos(tht[J1]+tht[J2]);
}

// *** æ[¬x ***
void endVelocity( Jaco, dTht, dPos )
	MAT *Jaco;
	double *dTht;
	double *dPos;
{
	register int i, j;
	int mm, nn;

	mm = MATheight( *Jaco );
	nn = MATwidth(  *Jaco );

	for( i=0; i<mm; i++ ){
		dPos[i] = 0.0;
		for( j=0; j<nn; j++ ){
			dPos[i] +=(MATelem(*Jaco,i+1,j+1) * dTht[j]);
		}
	}
}

// *** æ[¬xÌÚWüÍl ***
int demandInput( cnum, kP, Pint, Pend, len, time, lastTime, tht, dP )
	int cnum;
	double kP;
	double *Pint, *Pend;
	double *len;
	double time, lastTime;
	double *tht;
	double *dP;
{
	register int i;
	double str;
	double cxyz[3];
	double Amp;
	double stD, veD;
	double *p, *q;

	if( (p = (double *)calloc(cnum, sizeof(double))) == NULL ){
		return FAULT;
	}
	if( (q = (double *)calloc(cnum, sizeof(double))) == NULL ){
		return FAULT;
	}

 	str = 1.0;

	for( i=0; i<cnum; i++ ){
		cxyz[i]= (Pend[i] - Pint[i])/str;
	}

	// 3½®ÅCO¹ð¶¬
	Amp = 6.0*str/pow(lastTime, 2.0);
	veD = Amp*time - Amp*pow(time, 2.0)/(lastTime);
	stD = Amp*pow(time, 2.0)/2.0 - Amp*pow(time, 3.0)/(lastTime*3.0); 

	endPosition( len, tht, p );
	for( i=0; i<cnum; i++ ){
		 q[i] = Pint[i] + stD * cxyz[i];
		dP[i] = veD * cxyz[i] + kP*(q[i] - p[i]);
	}
	free(p);
	free(q);

	return TRUE;
}

// *** Ößp¬x ***
int jointRotateVelocity( Jaco, dPos, dTht )
	MAT *Jaco;
	double *dPos;
	double *dTht;
{
	register int i, j;
	int cnum, jnum;
	MAT invJaco;  // JÌtsñ

	cnum = MATheight(*Jaco);
	jnum = MATwidth(*Jaco);

	if( MATalloc( &invJaco, jnum, cnum ) == FAULT ){
		return( FAULT );
	}
	if( MATinvgauss(&invJaco, Jaco) == FAULT ){
		fprintf(stderr, "Error on p_inverse of Jacobian Matrix\n");
		exit(777);
	}
	for( i=0; i<jnum; i++ ){
		dTht[i] = 0.0;
		for( j=0; j<cnum; j++ ){
			dTht[i] += MATelem(invJaco, i+1, j+1) * dPos[j];
		}
	}
	MATfree( &invJaco );

	return TRUE;
}


// **************************
// *** sñvZCu ***
// **************************
#define swap(x,y) {double t; t = x; x = y; y = t;}

double MATeps = 1.0e-6;
int	maxNR = 500;

// *** zñÌmÛ ***
int MATalloc( MAT *mat, int m, int n )
{
	if( (mat->x = (double *)calloc(m*n, sizeof(double))) != NULL ){
		MATheight(*mat) = m;
		MATwidth( *mat) = n;
		return (TRUE);
	}
	MATwidth(*mat) = MATheight(*mat) = 0;
	return (FAULT);
}

// *** ÌJú ***
void MATfree( MAT *mat )
{
	free( mat->x );
}

// *** PÊsñÌ¶¬ ***
void MATsquare( MAT *mat )
{
	register int i, j;

	if( MATwidth(*mat) == MATheight(*mat) ){
		for( i=1; i<=MATheight(*mat); i++ ){
			for( j=1; j<=MATwidth(*mat); j++ ){
				MATelem(*mat,i,j) = 0.0;
			}
			MATelem(*mat,i,i) = 1.0;
		}
	}
}

// *** sñÌRs[i³ûsñÌêj***
void MATassign( MAT *mat1, MAT *mat2 )
{
	register int i, j;

	if( MATheight(*mat1) == MATheight(*mat2) &&
		MATwidth( *mat1) == MATwidth( *mat2)){
		for( i=1; i<=MATheight(*mat1); i++ ){
			for( j=1; j<=MATwidth(*mat1); j++ ){
				MATelem(*mat1,i,j) = MATelem(*mat2,i,j);
			}
		}
	}
}

// *** tsñÌvZ ***
int MATinvgauss( MAT *ia, MAT *x )
{
	register int i, j, k, n, mx;
	double temp;
	MAT a;

	n = MATheight(*ia);

	if( MATalloc(&a,n,n) == FAULT ){
		return FAULT;
	}
	MATassign( &a, x );
	MATsquare( ia );
	for( k=1; k<=n-1; k++ ){
		for( i=k+1, mx=k; i<=n; i++ ){
			if( fabs(MATelem(a,i,k)) > fabs(MATelem(a,mx,k)) ){
				mx = i;
			}
		}
		if( fabs(MATelem(a,mx,k)) < MATeps ){
			return FAULT;
		}
		if( mx != k ){
			for( j=k; j<=n; j++ ){
				swap(MATelem(a,k,j), MATelem(a,mx,j));
			}
			for( j=1; j<=n; j++ ){
				swap(MATelem(*ia,k,j), MATelem(*ia,mx,j));
			}
		}
		for( i=k+1; i<=n; i++ ){
			temp = MATelem(a,i,k) / MATelem(a,k,k);
			for( j=1; j<=n; j++ ){
				MATelem(a,i,j) -= temp * MATelem(a,k,j);
			}
			for( j=1; j<=n; j++ ){
				MATelem(*ia,i,j) -= temp * MATelem(*ia,k,j);
			}
		}
	}
	for( k=n; k>=1; k-- ){
		for( j=1; j<=n; j++ ){
			MATelem(*ia,k,j) /= MATelem(a,k,k);
		}
		for( i=k-1; i>=1; i-- ){
			for( j=1; j<=n; j++ ){
				MATelem(*ia,i,j) -= MATelem(*ia,k,j)*MATelem(a,i,k);
			}
		}
	}
	MATfree(&a);

	return TRUE;
}

// *** sñÌ\¦ ***
void MATdisp( MAT *mat )
{
	register int i, j;

	printf("\n");

	for( i=1; i<=MATheight(*mat); i++ ){
		for( j=1; j<=MATwidth(*mat); j++ ){
			printf("%f ", MATelem(*mat,i,j) );
		}
		printf("\n");
	}	
}

// *** ]usñÌ¶¬ ***
void MATtrans( MAT *ta, MAT *x )
{
	register int i, j;
	register int n, m;

	m = MATheight(*x);  // s
	n = MATwidth( *x);  // ñ

	for( i=1; i<=m; i++ ){
		for( j=1; j<=n; j++ ){
			MATelem(*ta,j,i) = MATelem(*x,i,j);
		}
	}
}

// *** sñÌÏ ***
void MATmulti( MAT *ma, MAT *mat1, MAT *mat2 )
{
	register int i, j, k;
	register int m1, n1, n2;

	m1 = MATheight(*mat1);  // mat1Ìs
	n1 = MATwidth( *mat1);  // mat1Ìñ = mat2Ìs
	n2 = MATwidth( *mat2);  // mat2Ìñ

	for( i=1; i<=m1; i++ ){
		for( j=1; j<=n2; j++ ){
			for( k=1; k<=n1; k++ ){
				MATelem(*ma,i,j) += MATelem(*mat1,i,k) * MATelem(*mat2,k,j);
			}
		}
	}
}

// *** [tsñÌvZ ***
int MATpse_invgauss( MAT *pia, MAT *x )
{
	// [tsñÌZo  pseudo-inverse matrix
	MAT JT;           // J^T (JÌ]u)
	MAT J_JT;         // JEJT
	MAT inv_J_JT;     // (JEJT)^1 (JEJTÌtsñj

	if( MATalloc( &JT, MATwidth(*x), MATheight(*x) ) == FAULT ){
		return( FAULT );
	}
	if( MATalloc( &J_JT, MATheight(*x), MATheight(*x) ) == FAULT ){
		return( FAULT );
	}
	if( MATalloc( &inv_J_JT, MATheight(*x), MATheight(*x) ) == FAULT ){
		return( FAULT );
	}

	// Jacobisñ Ì\¦
	//MATdisp(x);

	// JT (JÌ]usñ)ðßé
	MATtrans( &JT, x );  
	//MATdisp( &JT );

	// JEJT ðvZ
	MATmulti( &J_JT, x, &JT );
	//MATdisp( &J_JT );

	// JEJTÌtsñ ðßé
	MATinvgauss( &inv_J_JT, &J_JT );
	//MATdisp( &inv_J_JT );

	// JTE(JEJT)^1 (JÌ[tsñ)ðßé
	MATmulti( pia, &JT, &inv_J_JT );
	//MATdisp( pia );

	MATfree( &JT );
	MATfree( &J_JT );
	MATfree( &inv_J_JT );

	return TRUE;
}