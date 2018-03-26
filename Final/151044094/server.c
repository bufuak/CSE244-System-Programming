/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
   This version runs forever, forking off a separate 
   process for each connection
   gcc server2.c -lsocket
*/
#include <stdio.h>
#include <stdlib.h>   
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "matrix.h"
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>  
#include <errno.h>
#include <time.h>
#include <math.h>
#include <linux/limits.h>


#define FIFO_PERM (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define FIFO_MODES (O_RDONLY|O_WRONLY)

void* dostuff(void*); /* function prototype */
void* solve(void* arg);
void error(char *msg)
{
    perror(msg);
    exit(1);
}

sem_t   sem;
pthread_mutex_t mutex;

struct server
{
    int sockfd;
    int currently;
    pthread_t tid;
    int isThreadPool;
    pthread_t tids[255];
    int sockfds[255];
    int thpool_size;
};
typedef struct server Server;


int main(int argc, char *argv[])
{
   int sockfd, newsockfd, portno, clilen, pid,thpool_size;
   int isThreadPool;
   struct sockaddr_in serv_addr, cli_addr;
   sem_init(&sem,0,1);
   if (argc==2) {
    isThreadPool=0;
    thpool_size=-1;
   }
   else if(argc==3){
    thpool_size=atoi(argv[2]);
    isThreadPool=1;
   }
   else{
    fprintf(stderr,"Wrong parameters for server.\n./server portno <(workpool)thpool size,k>\n");
    exit(1);
   }
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) 
      error("ERROR opening socket");
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
     error("ERROR on binding");
   listen(sockfd,10);
   clilen = sizeof(cli_addr);
   int i=0;
   int j=0;
   if(isThreadPool==1)
   {
    Server server;
    server.currently=0;
    server.isThreadPool=isThreadPool;
    server.thpool_size=thpool_size;
    while(1)
    {
      printf("Server is currently serving %d clients\n",server.currently);
      newsockfd = accept(sockfd, 
           (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) 
        error("ERROR on accept");
      server.currently++;
      while(server.sockfds[i]!=0)
      {
        i++;
        if(i==thpool_size)
          i=0;
      }
      server.sockfds[i]=newsockfd;
      Server *serverptr=&server;
      int err=pthread_create(&server.tids[i],NULL,dostuff,serverptr);
      if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));          
    }
   }
   else
   {
    Server server;
    server.currently=0;
    server.isThreadPool=isThreadPool; 
    while(1)
    {
      newsockfd = accept(sockfd, 
           (struct sockaddr *) &cli_addr, &clilen);
      if (newsockfd < 0) 
        error("ERROR on accept");
      server.sockfd=newsockfd;
      server.currently++;
      printf("Server is currently serving %d clients\n",server.currently);
      Server *serverptr=&server;
      int err=pthread_create(&server.tid,NULL,dostuff,serverptr);
      if (err != 0)
        printf("\ncan't create thread :[%s]", strerror(err));
      usleep(50000);
    }
   }
}

void* dostuff (void* arg)
{
  Server* server;
  server=arg;
  int sock;
  int threadNo;
  if(server->isThreadPool==0)
  {
    sem_wait(&sem);
    sock=server->sockfd;
    sem_post(&sem); 
  }
  else
  {
    int i;
    for(i=0;i<server->thpool_size;i++)
    {
      if(server->tids[i]==pthread_self())
      {
        sock=server->sockfds[i];
        threadNo=i;
      }
    }
  }
  int n;
  int pid;
  long int tid;
  int m;
  int p;
  n = read(sock,&pid,sizeof(int));
  if (n < 0) error("ERROR reading from socket");
  n = read(sock,&tid,sizeof(long int));
  if (n < 0) error("ERROR reading from socket");
  n = read(sock,&m,sizeof(int));
  if (n < 0) error("ERROR reading from socket");
  n = read(sock,&p,sizeof(int));
  if (n < 0) error("ERROR reading from socket");
  int thispid=getpid();
  int childpid=fork();
  int grandchildpid=-1;
  if(childpid==0)
    grandchildpid=fork();

  if(thispid==getpid())
  {
    //printf("ENPARENTBENİM;Ben KONTROL edicem\n");
    char fifoName[255];
    sprintf(fifoName,"client%d-%ld",pid,(long int)tid);
    if(tid==0 || pid==0)
    {
      key_t key=childpid;
      int shmid;
      Matrix *matptr;
      if ((shmid = shmget(key, sizeof(Matrix), IPC_CREAT | 0666)) < 0) {
          perror("shmget");
          exit(1);
      }
      /*
       * Now we attach the segment to our data space.
       */
      if ((matptr = shmat(shmid, NULL, 0)) == (Matrix *) -1) {
          perror("shmat");
          exit(1);
      }

      sem_wait(&sem);
      server->currently--;
      sem_post(&sem);

      if(server->isThreadPool==1)
      {
        server->tids[threadNo]=0;
        server->sockfds[threadNo]=0;
      }

      shmctl(shmid,IPC_RMID,NULL);

    }
    else
    {
      int server_to_client=open(fifoName,O_WRONLY);
      char logName[255];
      sprintf(logName,"log/log_server%d-%ld",pid,tid);
      FILE* logPtr;
      struct stat st = {0};
      if (stat("./log", &st) == -1) {
          mkdir("./log", 0700);
      }
      logPtr=fopen(logName,"a+");
      

      key_t key=childpid;
      int shmid;
      Matrix *matptr;
      if ((shmid = shmget(key, sizeof(Matrix), IPC_CREAT | 0666)) < 0) {
          perror("shmget");
          exit(1);
      }
      /*
       * Now we attach the segment to our data space.
       */
      if ((matptr = shmat(shmid, NULL, 0)) == (Matrix *) -1) {
          perror("shmat");
          exit(1);
      }

      while(matptr->wait!=2)
      {
        //Matrix'in çözülmesini bekliyorum.
      }
    
      int i,j;
      /*
      printf("A:\n");
      for(i=0;i<matptr->m;i++)
      {
        for(j=0;j<matptr->p;j++)
          printf("%f ",matptr->A[i][j]);
        printf("\n");
      }
      printf("\n");

      printf("x:\n");
      for(i=0;i<matptr->p;i++)
        printf("%f ",matptr->x[i]);
      printf("\n");


      printf("b:\n");
      for(i=0;i<matptr->m;i++)
        printf("%f ",matptr->b[i]);
      printf("\n");
      
      */
      double error[m];
      double sum=0;
      for(i=0;i<matptr->m;i++)
      {
        for(j=0;j<matptr->p;j++)
        {
          sum=sum+matptr->A[i][j]*matptr->x[j];
        }
        error[i]=sum-matptr->b[i];
        sum=0;
      }

      /*
      printf("error:\n");
      for(i=0;i<matptr->m;i++)
        printf("%f ",error[i]);
      printf("\n");
      
      */
      matptr->error=0;
      for(i=0;i<matptr->m;i++)
        matptr->error+=error[i]*error[i];

      matptr->error=sqrt(matptr->error);

      write(server_to_client, matptr, sizeof(Matrix));

      int a,b;
      fprintf(logPtr,"Matrix is:\n[");
      for(a=0;a<matptr->m;a++)
      {
        for(b=0;b<matptr->p;b++)
        {
          fprintf(logPtr,"%.2f",matptr->A[a][b]);
          if(b==matptr->p-1)
          {
            if(a==matptr->m-1)
            {
              fprintf(logPtr,"]\n");
            }
            else
            {
              fprintf(logPtr,";\n");  
            }
          }
          else
          {
            fprintf(logPtr,",");
          }
          //printf("%.2f  ",mat.matrix[a][b]);

        }
        //printf("\n");
      }
      fprintf(logPtr,"\n");
      fprintf(logPtr,"b is:\n[");
      {
        for(a=0;a<matptr->m;a++)
        {
          fprintf(logPtr,"%.2f",matptr->b[a]);
          if(a==matptr->m-1)
          {
            fprintf(logPtr,"]\n");
          }
          else
          {
            fprintf(logPtr,",");
          }
        }
      }
      fprintf(logPtr,"\n");


      fprintf(logPtr,"solution is:\n[");
      {
        for(a=0;a<matptr->p;a++)
        {
          fprintf(logPtr,"%.2f",matptr->x[a]);
          if(a==matptr->p-1)
          {
            fprintf(logPtr,"]\n");
          }
          else
          {
            fprintf(logPtr,",");
          }
        }
      }
      fprintf(logPtr,"\n\nError is: %f",matptr->error);

      fclose(logPtr);
      

      sem_wait(&sem);
      server->currently--;
      sem_post(&sem);

      if(server->isThreadPool==1)
      {
        server->tids[threadNo]=0;
        server->sockfds[threadNo]=0;
      }

      shmctl(shmid,IPC_RMID,NULL);
      close(server_to_client);

    }

    while(wait(NULL)>0)
    {
      //Zombie bırakmamak için.Processler paralel çalışıyor.
    }
    return;
  }

  else if(grandchildpid>0)
  {
    key_t key=grandchildpid;
    int shmid;
    Matrix *matptr;
    if ((shmid = shmget(key, sizeof(Matrix), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /*
     * Now we attach the segment to our data space.
     */
    if ((matptr = shmat(shmid, NULL, 0)) == (Matrix *) -1) {
        perror("shmat");
        exit(1);
    }

    while(matptr->wait!=1)
    {
      // Matrixin generate edilmesini bekliyoruz.
    }
  
    int i;
    for(i=0;i<3;i++)
    {
        int err = pthread_create(&(matptr->tid[i]), NULL, &solve,matptr);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
    }
    for(i=0;i<3;i++)
    {
        pthread_join(matptr->tid[i],NULL);  
    }

    int key2=getpid();
    Matrix *mat2ptr;
    int shmid2;
    if ((shmid2 = shmget(key2, sizeof(Matrix), IPC_CREAT|0666)) < 0) {
        perror("shmget");
        exit(1);
    }
    /*
     * Now we attach the segment to our data space.
     */
    if ((mat2ptr = shmat(shmid2, NULL, 0)) == (Matrix *) -1) {
        perror("shmat");
        exit(1);
    }

    int j;
    mat2ptr->m=matptr->m;
    mat2ptr->p=matptr->p;
    for(i=0;i<matptr->m;i++)
    {
      for(j=0;j<matptr->p;j++)
      {
        mat2ptr->A[i][j]=matptr->A[i][j];
      }
    }
    for(i=0;i<matptr->m;i++)
      mat2ptr->b[i]=matptr->b[i];

    for(i=0;i<matptr->p;i++)
      mat2ptr->x[i]=matptr->x[i];

    mat2ptr->wait=2; // Matrix sistemi çözüldü.

    shmctl(shmid,IPC_RMID,NULL);
    //printf("PARENTIN OĞLU BENİM;Ben sonuç bulacam\n");
    while(wait(NULL)>0)
    {
      //Zombie bırakmamak için. Processler paralel çalışıyor.
    }
    exit(1);
  }
  else
  {
    key_t key=getpid();
    int shmid;
    Matrix *matptr;
    if ((shmid = shmget(key, sizeof(Matrix), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((matptr = shmat(shmid, NULL, 0)) == (Matrix *) -1) {
        perror("shmat");
        exit(1);
    }

    matptr->m=m;
    matptr->p=p;
    int i,j;
    for(i=0;i<m;i++)
    {
      for(j=0;j<p;j++)
      {
          matptr->A[i][j]=(abs(getpid()*rand())%20)-9;
      }
    }

    for(j=0;j<m;j++)
    {
      matptr->b[j]=(abs(getpid()*rand())%20)-9;
    }
    matptr->wait=1; // Matrix generate edildi

    //printf("EN TORUN BENİM;BEN GENERATE EDECEM\n");
    exit(1);
  }
}

void* solve(void* arg)
{
  Matrix* mptr=arg;

  if(mptr->tid[0]==pthread_self())
  {
    //psudo
    Matrix at;
    at.m=mptr->p;
    at.p=mptr->m;

    Matrix ata;
    ata.m=mptr->p;
    ata.p=mptr->p;

    Matrix ataat;
    ataat.m=mptr->p;
    ataat.p=mptr->m;

    int i,j,k;
    for(i=0;i<mptr->m;i++)
    {
      for(j=0;j<mptr->p;j++)
      {
        at.A[j][i]=mptr->A[i][j];
      }
    }
    double sum=0;
    for(i=0;i<ata.m;i++)
    {
      for(j=0;j<ata.p;j++)
      {
        for(k=0;k<at.p;k++)
        {
          //printf("At[%d][%d]:%f * A[%d][%d]:%f\n",i,k,at.A[i][k],k,j,mptr->A[k][j]);
          sum=sum+at.A[i][k]*mptr->A[k][j];
        }
        ata.A[i][j]=sum;
        //printf("Ata[%d][%d]sum:%f\n",i,j,ata.A[i][j]);
        sum=0;
      }
    }

    Matrix *ataptr=&ata;
    cofactor(ataptr,ataptr->m);
  
    for(i=0;i<ataat.m;i++)
    {
      for(j=0;j<ataat.p;j++)
      {
        for(k=0;k<at.m;k++)
        {
          //printf("At[%d][%d]:%f * A[%d][%d]:%f\n",i,k,at.A[i][k],k,j,mptr->A[k][j]);
          sum=sum+ata.A[i][k]*at.A[k][j];
        }
        ataat.A[i][j]=sum;
        //printf("Ata[%d][%d]sum:%f\n",i,j,ata.A[i][j]);
        sum=0;
      }
    }

    for(i=0;i<mptr->p;i++)
    {
      for(k=0;k<mptr->m;k++)
      {
        sum=sum+ataat.A[i][k]*mptr->b[k];
      }
      mptr->x[i]=sum;
      sum=0;
    }

  }
  else if(mptr->tid[1]==pthread_self())
  {
    //qr
  }
  else
  {
    //svd
  }
  
  return; 
}
/*
int i,j;
    for(i=0;i<m;i++)
    {
      for(j=0;j<p;j++)
      {
          printf("%d ",matptr->A[i][j]);
      }
      printf("\n");
    }
    printf("\n");
    for(j=0;j<p;j++)
    {
      printf("%d ",matptr->b[j]);
    }
    printf("\n");
    
    */
