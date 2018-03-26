
#include "matrix.h"
void seeWhat(char * mainPipeName);


static volatile sig_atomic_t signal_flag=0;
static volatile sig_atomic_t program_flag=0;
static volatile sig_atomic_t exit_flag=0;

static void setExitFlag(int signo){
	exit_flag=1;
}
static void notExit(int signo){
	exit_flag=0;
}
static void setSignalFlag(int signo){
	signal_flag=1;
}
static void setProgramFlag(int signo){
	program_flag=1;
}


int main(int argc,char * argv[])
{
	seeWhat(argv[1]);
	return 0;
}

void seeWhat(char * mainPipeName)
{
	FILE *dumb,*dumb1,*dumb2;
	FILE *logPtr;
	int server_to_client;
	int client_to_server;
	int server_to_client_matrix;
	int client_to_results;
	int fd[2];
	int fd2[2];
	int serverPid;
	int thisPid;
	int readBytes;
	int convolutionType;
	int showResultsPid=0;
	char logName[255];
	char secondPipe[255];
	char thirdPipe[255];
	char * showResultsFifo="showResultsFifo";
	double convMatrix[3][3];
	Matrix mat;
	struct timespec tpend,tpstart;
	struct sigaction act;
	
	act.sa_handler=setExitFlag;
	act.sa_flags=0;
	if((sigemptyset(&act.sa_mask)==-1)||
		(sigaction(SIGINT,&act,NULL)==-1))
	{
		perror("Failed to set SIGINT handler\n");
		return;
	}
	act.sa_handler=notExit;
	act.sa_flags=1;
	if(sigaction(SIGQUIT,&act,NULL)==-1||
		sigaction(SIGTSTP,&act,NULL)==-1)
	{
		perror("Failed to set SIGQUIT and SIGTSTP\n");
		return;
	}

	act.sa_handler=setSignalFlag;
	act.sa_flags=3;
	if(sigaction(SIGUSR1,&act,NULL)==-1)
	{
		perror("Failed to set SIGUSR1\n");
		return;
	}

	act.sa_handler=setProgramFlag;
	act.sa_flags=3;
	if(sigaction(SIGUSR2,&act,NULL)==-1)
	{
		perror("Failed to set SIGUSR2\n");
		return;
	}

	struct stat st = {0};

	if (stat("./log", &st) == -1) {
	    mkdir("./log", 0700);
	}
	
	thisPid=getpid();

	if(mkfifo(showResultsFifo,FIFO_PERM) < 0) {
	    if(errno!=EEXIST)
		{
			fprintf(stderr,"(%ld):failed to create named pipe %s: %s\n",(long)getpid(),secondPipe,strerror(errno));
			exit(-1);
		}
  	}
  	dumb1=fopen(showResultsFifo,"a+");
  	if((client_to_results = open(showResultsFifo,O_WRONLY)) < 0) 
    {
    	perror(showResultsFifo);
    	exit(1);
    } 
	

	 /*Opening PUBLIC fifo in READ ONLY mode*/
    if((server_to_client = open(mainPipeName,O_RDONLY)) < 0) 
    {
    	fprintf(stderr,"There is no timerServer running with the fifoname,%s\n",mainPipeName);
    	exit(1);
    }  
	int somenumber=read(server_to_client, &serverPid, sizeof(serverPid));
	close(server_to_client);

	//
	
	sprintf(secondPipe,"server%d",serverPid);
	if((client_to_server = open(secondPipe,O_WRONLY)) < 0) 
	{
		perror(secondPipe);
		exit(1);
	}
	write(client_to_server,&thisPid,sizeof(thisPid));
	close(client_to_server);

	
	sprintf(thirdPipe,"client%d",thisPid);
	if(mkfifo(thirdPipe,FIFO_PERM) < 0) {
	    if(errno!=EEXIST)
		{
			fprintf(stderr,"(%ld):failed to create named pipe %s: %s\n",(long)getpid(),secondPipe,strerror(errno));
			exit(-1);
		}
  	}
  	dumb=fopen(thirdPipe,"a+");

	if((server_to_client_matrix = open(thirdPipe,O_RDONLY)) < 0) 
	{
		perror(thirdPipe);
		exit(1);
	}


	int convCondition=1;
	while(convCondition)
	{
		printf("Choose 2d convolution type:\n");
		printf("Press \n1 to Kernel matrix:\n0 0 0\n0 1 0\n0 0 0\n");
		printf("Press \n2 to Skew-symetric matrix1:\n 1 1-1\n-1 1-1\n 1 1 1\n");
		printf("Press \n3 to Skew-symetric matrix2:\n 0 1-1\n-1 0-1\n 1 1 0\n");
		scanf("%d",&convolutionType);
		switch(convolutionType)
		{
			case 1: convMatrix[0][0]=0;convMatrix[0][1]=0;convMatrix[0][2]=0;
					convMatrix[1][0]=0;convMatrix[1][1]=1;convMatrix[1][2]=0;
					convMatrix[2][0]=0;convMatrix[2][1]=0;convMatrix[2][2]=0;
					convCondition=0;
					break;
			case 2: convMatrix[0][0]=1;convMatrix[0][1]=1;convMatrix[0][2]=1;
					convMatrix[1][0]=-1;convMatrix[1][1]=1;convMatrix[1][2]=-1;
					convMatrix[2][0]=-1;convMatrix[2][1]=1;convMatrix[2][2]=1;
					convCondition=0;
					break;
			case 3: convMatrix[0][0]=0;convMatrix[0][1]=1;convMatrix[0][2]=1;
					convMatrix[1][0]=-1;convMatrix[1][1]=0;convMatrix[1][2]=-1;
					convMatrix[2][0]=-1;convMatrix[2][1]=1;convMatrix[2][2]=0;
					convCondition=0;
					break;
			default: printf("Please enter valid choice of 2d convolution type.\n");
					break;
		}
	}
	
	//printf("Server's pid is:%d\n",serverPid);
	//printf("I know server's pid now requesting matrix\n");
	kill(serverPid,SIGUSR1); // Say to server "I am alive!"
	int counter=0;
	int condition=-1;
	while(!exit_flag)
	{
		condition++;
		//printf("%d\n"	,getpid());
	  	if(program_flag==1)
	  	{
	  		while(wait(NULL)>0)
	  		{

	  		}
	  		sprintf(logName,"log/logClose_seeWhat%d",getpid());
			logPtr=fopen(logName,"a+");
			fprintf(logPtr,"timerServer or showResults is dead. I'll kill myself\n");
			kill(serverPid,SIGUSR2);
			kill(showResultsPid,SIGUSR2);
			fclose(logPtr);
	  		close(server_to_client_matrix);
	  		close(client_to_results);
	  		unlink(showResultsFifo);
			unlink(thirdPipe);
			return;
	  	}
	  	if(thisPid==getpid())
	  	{
	  		readBytes = read(server_to_client_matrix,&mat,sizeof(mat));
	  		if(readBytes>0) // New client;
			{
				sprintf(logName,"log/log%d_seeWhat%d.txt",condition,getpid());
	  			logPtr=fopen(logName,"a+");
				//printf("condition:%d,check :%d,readBytes %d getpid:%d\n",condition,check,readBytes,getpid());
				pipe(fd);
				pipe(fd2);
				int a,b,c;
				c=0;
				if(condition==counter)	
					fprintf(logPtr,"Matrix is:\n[");
				for(a=0;a<mat.size;a++)
				{
					for(b=0;b<mat.size;b++)
					{
						if(condition==counter)
							fprintf(logPtr,"%.2f",mat.matrix[a][b]);
						if(b==mat.size-1)
						{
							if(a==mat.size-1)
							{
								if(condition==counter)
									fprintf(logPtr,"]");
							}
							else
							{
								if(condition==counter)
									fprintf(logPtr,";\n");	
							}
						}
						else
						{
							if(condition==counter)
								fprintf(logPtr,",");
						}
						//printf("%.2f 	",mat.matrix[a][b]);

					}
					//printf("\n");
				}
				if(condition==counter)
					fprintf(logPtr,"\n");
				fclose(logPtr);
				//printf("Matrix's order is:%d,determinant is:%f\n",mat.size,mat.determinant);
				counter++;
				fork();
				if(thisPid!=getpid())
					fork();
		  	}
		}

		if(thisPid==getpid()) // Parent waits for children
		{
			mat.size=0;
			Result r1,r2;
			ClientResult cr;
			while(wait(NULL)>0)
			{

			}
			close(fd[1]);
			close(fd2[1]);
			read(fd[0],&r1,sizeof(r1));
			read(fd2[0],&r2,sizeof(r2));
			close(fd2[0]);
			close(fd[0]);
			//printf("pResult1 is:%f,time is:%ld\n",r1.result,r1.time);
			//printf("pResult2 is:%f,time is:%ld\n",r2.result,r2.time);
			cr.pid=getpid();
			cr.r1=r1;
			cr.r2=r2;
			write(client_to_results, &cr, sizeof(cr));
			//printf("Parent:%d\n\n\n\n",getpid());
		}
		else if(thisPid==getppid()) // 2D Convolution
		{
			while(wait(NULL)>0) // Bu wait resultların sırayla pipe'a yazılmasını sağlayacak
			{
				//
			}
			logPtr=fopen(logName,"a+");
			Result r2;
			if(clock_gettime(CLOCK_REALTIME,&tpstart)==-1)
			{
				perror("Failed to get starting time\n");
				exit(-1);
			}
			//printf("CHILD:%d\n",getpid());
			Matrix convolved;
			convolved.size=mat.size;
		    int i,j,k,l,m,n;
		    double sum;
		    for(i=0;i<mat.size;i++)
		    {
		    	for(j=0;j<mat.size;j++)
		    	{	
		    		sum=0;
		    		m=0;
		    		for(k=i-1;k<i+2;k++)
		    		{
		    			n=0;
		    			for(l=j-1;l<j+2;l++)
		    			{
		    				if(k<0||l<0||k>mat.size||l>mat.size)
		    				{
		    					//printf("i:%d,j:%d:k:%d,l:%d:m:%d,n:%d\n",i,j,k,l,m,n);
		    					//printf("0 x %f\n",convMatrix[m][n]);

		    					sum+=0;
		    				}
		    				else
		    				{
		    					//printf("i:%d,j:%d:k:%d,l:%d:m:%d,n:%d\n",i,j,k,l,m,n);
		    					//printf("%f x %f\n",matrix.matrix[k][l],convMatrix[m][n]);
		    					sum+=mat.matrix[k][l]*convMatrix[m][n];
		    				}
		    				n++;
		    			}
		    			m++;
		    			//printf("\nSum is:%f\n",sum);
		    		}
		    		convolved.matrix[i][j]=sum;
		    	}
		    }
		    convolved.determinant=determinant(convolved.matrix,convolved.size);
		    //printf("Conv matrix determinant is:%f\n",convolved.determinant);
		    r2.result=mat.determinant-convolved.determinant;
		    fprintf(logPtr,"Convulution is:\n[");
		    for(i=0;i<mat.size;i++)
			{
				for(j=0;j<mat.size;j++)
				{
					fprintf(logPtr,"%.2f",convolved.matrix[i][j]);
					if(j==mat.size-1)
					{
						if(i==mat.size-1)
							fprintf(logPtr,"]");
						else
						{
							fprintf(logPtr,";\n");	
						}
					}
					else
						fprintf(logPtr,",");
					//printf("%.2f 	",convolved.matrix[i][j]);
				}
				//printf("\n");
			}
			//printf("\n");
			fprintf(logPtr,"\n");
			close(server_to_client_matrix);
			if(clock_gettime(CLOCK_REALTIME,&tpend)==-1)
			{
				perror("Faiiled to get ending time\n");
				exit(-1);
			}
			r2.time=MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
			//printf("cResult2 is:%f, time elapsed:%ld\n",r2.result,r2.time);
			close(fd[0]);
			close(fd2[0]);
			write(fd2[1],&r2,sizeof(r2));
			close(fd2[1]);
			close(fd[1]);
			fclose(logPtr);
			fclose(dumb);
			fclose(dumb1);
			close(client_to_results);
			exit(-1);
		}
		else // Shifted inverse
		{
			//printf("GRANDCHILD:%d\n",getpid());
			int i,j,k,l,m,n;
			Result r1;
			logPtr=fopen(logName,"a+");
			Matrix m1,m2,m3,m4;
			m1.size=mat.size/2;
			m2.size=mat.size/2;
			m3.size=mat.size/2;
			m4.size=mat.size/2;

			if(clock_gettime(CLOCK_REALTIME,&tpstart)==-1)
			{
				perror("Failed to get starting time\n");
				exit(-1);
			}
			
			for(i=0;i<mat.size/2;i++)
			{
				for(j=0;j<mat.size/2;j++)
				{
					m1.matrix[i][j]=mat.matrix[i][j];
				}
			}
			k=0;
			for(i=0;i<mat.size/2;i++)
			{
				l=0;
				for(j=mat.size/2;j<mat.size;j++)
				{
					m2.matrix[k][l++]=mat.matrix[i][j];
				}
				k++;
			}
			k=0;
			for(i=mat.size/2;i<mat.size;i++)
			{
				int l=0;
				for(j=0;j<mat.size/2;j++)
				{
					m3.matrix[k][l++]=mat.matrix[i][j];
				}
				k++;
			}
			k=0;
			for(i=mat.size/2;i<mat.size;i++)
			{
				int l=0;
				for(j=mat.size/2;j<mat.size;j++)
				{
					m4.matrix[k][l++]=mat.matrix[i][j];
				}
				k++;
			}

			Matrix *m1ptr=&m1;
			Matrix *m2ptr=&m2;
			Matrix *m3ptr=&m3;
			Matrix *m4ptr=&m4;
			cofactor(m1ptr,m1.size);
			cofactor(m2ptr,m2.size);
			cofactor(m3ptr,m3.size);
			cofactor(m4ptr,m4.size);

			Matrix shiftedInverse;
			shiftedInverse.size=mat.size;

			for(i=0;i<mat.size/2;i++)
			{
				for(j=0;j<mat.size/2;j++)
				{
					shiftedInverse.matrix[i][j]=m1.matrix[i][j];
				}
			}
			k=0;
			for(i=0;i<mat.size/2;i++)
			{
				l=0;
				for(j=mat.size/2;j<mat.size;j++)
				{
					shiftedInverse.matrix[i][j]=m2.matrix[k][l++];
				}
				k++;
			}
			k=0;
			for(i=mat.size/2;i<mat.size;i++)
			{
				int l=0;
				for(j=0;j<mat.size/2;j++)
				{
					shiftedInverse.matrix[i][j]=m3.matrix[k][l++];
				}
				k++;
			}
			k=0;
			for(i=mat.size/2;i<mat.size;i++)
			{
				int l=0;
				for(j=mat.size/2;j<mat.size;j++)
				{
					shiftedInverse.matrix[i][j]=m4.matrix[k][l++];
				}
				k++;
			}

			shiftedInverse.determinant=determinant(shiftedInverse.matrix,shiftedInverse.size);
		    //printf("Shifted inverse matrix determinant is:%f\n",shiftedInverse.determinant);
		    r1.result=mat.determinant-shiftedInverse.determinant;

			fprintf(logPtr,"Shifted inverse is:\n[");
		    for(i=0;i<mat.size;i++)
			{
				for(j=0;j<mat.size;j++)
				{
					fprintf(logPtr,"%.2f",shiftedInverse.matrix[i][j]);
					if(j==mat.size-1)
					{
						if(i==mat.size-1)
							fprintf(logPtr,"]");
						else
						{
							fprintf(logPtr,";\n");	
						}
					}
					else
						fprintf(logPtr,",");
					//printf("%.2f 	",shiftedInverse.matrix[i][j]);
				}
				//printf("\n");
			}
			//printf("\n");
			fprintf(logPtr,"\n");

			if(clock_gettime(CLOCK_REALTIME,&tpend)==-1)
			{
				perror("Faiiled to get ending time\n");
				exit(-1);
			}
			r1.time=MILLION*(tpend.tv_sec-tpstart.tv_sec)+(tpend.tv_nsec-tpstart.tv_nsec)/1000;
			//printf("sResult1 is:%f, time elapsed:%ld\n",r1.result,r1.time);
			
			close(fd2[0]);
			close(fd[0]);
			write(fd[1],&r1,sizeof(r1));
			close(fd[1]);
			close(fd2[1]);
			close(server_to_client_matrix);
			close(client_to_results);
			fclose(logPtr);
			fclose(dumb);
			fclose(dumb1);
			exit(-1);
		}
	}

	sprintf(logName,"log/logClose_seeWhat%d",getpid());
	logPtr=fopen(logName,"a+");
	fprintf(logPtr,"CTRL + C pressed! Closing seeWhat\n");
	fclose(logPtr);
	close(server_to_client_matrix);
	close(client_to_results);
	fclose(dumb1);
	fclose(dumb);
	unlink(thirdPipe);
	unlink(showResultsFifo);
 	kill(serverPid,SIGUSR2);
	kill(showResultsPid,SIGUSR2);
 	return;
}