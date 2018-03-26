#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>	
#include <errno.h>
#include <time.h>
#include <math.h>
#include <linux/limits.h>

#define FIFO_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define FIFO_MODES (O_RDONLY|O_WRONLY)


#define maxSize 20
#define MILLION 1000000L

struct result
{
	double result;
	long time;
};

typedef struct result Result;

struct clientResult
{
	int pid;
	Result r1;
	Result r2;	
};

typedef struct clientResult ClientResult;

struct matrix
{
	int size;
	double matrix[maxSize][maxSize];
	double determinant;
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
		matrix->matrix[0][0]=1.0/matrix->matrix[0][0];
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
		            	b[m][n]=matrix->matrix[i][j];
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
	d=determinant(matrix->matrix,matrix->size);
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
	    	matrix->matrix[i][j]=inverse[i][j];
	    }
	    printf("\n");
	}
}

void randomInvertibleMatrix(Matrix* mat)
{
	int mSize=mat->size;
	int row,col;
	for(row=0;row<mSize;row++)
	{
		for(col=0;col<mSize;col++)
		{
			mat->matrix[row][col]=(abs(getpid()*rand())%20)-9;
		}
	}
	if(determinant(mat->matrix,mat->size)==0.0)
	{
		randomInvertibleMatrix(mat);
	}
	else
		return;
}

#endif