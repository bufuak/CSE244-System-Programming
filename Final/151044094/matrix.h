#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
struct matrix
{
  int m;
  int p;
  double A[20][20];
  double b[20];
  double x[20];
  double error;
  int wait;
  pthread_t tid[3];
};
typedef struct matrix Matrix;

double determinant(double[20][20],int mSize);
void cofactor(Matrix *matrix,int mSize);
void transpose(Matrix *matrix,double fac[20][20],int r);
void randomInvertibleMatrix(Matrix * mat);

/*For calculating Determinant of the Matrix */
double determinant(double a[20][20],int mSize)
{
	double s=1,det=0,b[20][20];
    int i,j,m,n,c;
    if (mSize==1)
    {
    	return (a[0][0]);
    }
    else
    {
    	det=0;
    	for (c=0;c<mSize;c++)
    	{
	        m=0;
	        n=0;
	        for (i=0;i<mSize;i++)
	        {
	            for (j=0;j<mSize;j++)
	            {
	                b[i][j]=0;
	                if (i != 0 && j != c)
	                {
	                    b[m][n]=a[i][j];
	                    if (n<(mSize-2))
	                     	n++;
	                    else
	                    {
	                    	n=0;
	                    	m++;
	                    }
	                }
	            }
	        }
	        det=det + s * (a[0][c] * determinant(b,mSize-1));
	        s=-1 * s;
        }
    }
    return (det);
}

void cofactor(Matrix *matrix,int mSize)
{
	if(mSize==1)
	{
		matrix->A[0][0]=1.0/matrix->A[0][0];
		return;
	}
	double b[20][20],fac[20][20];
	int p,q,m,n,i,j;
	for (q=0;q<mSize;q++)
	{
	    for (p=0;p<mSize;p++)
	    {
		    m=0;
		    n=0;
		    for (i=0;i<mSize;i++)
		    {
		       for (j=0;j<mSize;j++)
		       {
		            if (i != q && j != p)
		            {
		            	b[m][n]=matrix->A[i][j];
		           		if (n<(mSize-2))
		                	n++;
		            	else
		             	{
		               		n=0;
		               		m++;
		                }
		            }
		        }
		    }
		    fac[q][p]=pow(-1,q + p) * determinant(b,mSize-1);
	    }
	}
	transpose(matrix,fac,mSize);
}
/*Finding transpose of matrix*/ 
void transpose(Matrix *matrix,double fac[20][20],int r)
{
	int i,j;
	double b[20][20],inverse[20][20],d;
	 
	for (i=0;i<r;i++)
	{
	    for (j=0;j<r;j++)
	    {
	        b[i][j]=fac[j][i];
	    }
	}
	d=determinant(matrix->A,matrix->p);
	for (i=0;i<r;i++)
	{
	   	for (j=0;j<r;j++)
	    {
	    	inverse[i][j]=b[i][j] / d;
	    }
	}
	 
	for (i=0;i<r;i++)
	{
	    for (j=0;j<r;j++)
	    {
	    	matrix->A[i][j]=inverse[i][j];
	    }
	}
}


#endif