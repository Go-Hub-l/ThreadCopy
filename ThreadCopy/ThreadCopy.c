/*利用mmap 函数实现 多线程拷贝文件*/

/*默认创建5个线程*/

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


//全局变量
int fileSize = 0;
int ThreadNum = 5;//默认创建5个线程
int fd;//共享文件描述符

#define READSIZE 20 

typedef struct CopyInfo
{
	int count;
	char *des_file;
	char *src_file;
}CopyInfo;

//线程的拷贝函数
void *Thread_Copy(void *Copy)
{
	CopyInfo *pCopy = (CopyInfo*)Copy;
	int fin = open(pCopy->src_file,O_RDONLY);
	if(fin == -1)
	{
		perror("open src_file error");
		exit(0);
	}
	int fout = open(pCopy->des_file,O_RDWR|O_CREAT,0664);
	if(fout == -1)
	{
		perror("open des_file error");
		exit(0);
	}
	//为每个线程建立文件映射
	char *In_ptr = mmap(NULL,fileSize,PROT_READ,MAP_SHARED,fin,0);
	char *Out_ptr = mmap(NULL,fileSize,PROT_READ|PROT_WRITE,MAP_SHARED,fout,0);
	//拓宽文件
	ftruncate(fout,fileSize);
	//检验映射
	if(In_ptr == MAP_FAILED || Out_ptr == MAP_FAILED)
	{
		perror("call mmap failed");
		close(fin);
		close(fout);
		exit(0);
	}
	/*------------------------线程拷贝------------------------------*/
	//计算每个线程拷贝的大小
	int wsize = 0;
	wsize = fileSize/ThreadNum;
	//如果不能整除，则将wsize+1
	wsize = fileSize%ThreadNum ? wsize+1:wsize;
	//偏移指针位置
	int wsite = pCopy->count*wsize;
	//统计线程拷贝的子数
	int flag = 0;
	while(flag < wsize && In_ptr[wsite+flag] != EOF)
	{
		Out_ptr[wsite+flag] = In_ptr[wsite+flag];
		flag++;
	}

	//线程拷贝完成后释放空间和文件描述符
	close(fin);
	close(fout);

	free(pCopy);
	pCopy = NULL;
	//取消映射
	munmap(In_ptr,fileSize);
	munmap(Out_ptr,fileSize);
}

void Bar(char *arg)
{
	//打印进度条
	while(1)
	{
		if(fileSize == 0)
			continue;
		//退格，回到开头
		printf("\r");
		//打开拷贝的文件
		int rfd = open(arg,O_RDONLY);
		if(rfd == -1)
			continue;
		//计算已经拷贝的字节数
		int copyNum = lseek(rfd,0,SEEK_END);
		//打印进度条
		int na = 0;
		int ns = 0;
		printf("[");
		na = copyNum*1.0/fileSize * 100;
		ns = 100 - na;
		while(na--)
		{
			printf("=");
		}
		while(ns--)
		{
			printf(" ");
		}
		//拷贝百分比
		printf("][%.2f%%]",copyNum*1.0/fileSize * 100);
		if(copyNum >= fileSize)
		{
			printf("\n");
			return;
		}
		close(rfd);
		fflush(NULL);
		usleep(100000);
	}
}
int main(int argc,char **argv)
{
	if(argc < 3)
	{
		printf("argument defect\n");
		exit(0);
	}


	//检测、更新传入线程数
	if(argc == 4)
	{
		ThreadNum = atoi(argv[3]);
		if(ThreadNum <=0 || ThreadNum > 100)
		{
			printf("ThreadNum Input Error:1~100\n");
			exit(0);
		}
	}
	//打开共享文件
	int fd = open(argv[1],O_RDONLY);
	//计算文件大小
	fileSize = lseek(fd,0,SEEK_END);
	//记录每个线程的tid
	pthread_t *tid = NULL;
	tid = (pthread_t*)malloc(sizeof(pthread_t)*ThreadNum);
	//信息结构体
	CopyInfo **pCopy = (CopyInfo**)malloc(sizeof(CopyInfo*)*ThreadNum);
	int err;
	for(int i = 0;i < ThreadNum;i++)
	{
		CopyInfo *pTemp = (CopyInfo*)malloc(sizeof(CopyInfo));

		pTemp->src_file = argv[1];
		pTemp->des_file = argv[2];
		pTemp->count = i;

		pCopy[i] = pTemp;
		//创建线程
		if((err = pthread_create(&tid[i],NULL,Thread_Copy,(void *)pCopy[i])) > 0)
		{

			printf("pthread_create copy error:%s\n",strerror(err));
			exit(0);
		}
	}
	//打印进度条
	Bar(argv[2]);
	//回收线程
	for(int i = 0;i < ThreadNum;i++)
	{
		int err = pthread_join(tid[i],NULL);

		if(err > 0)
			printf("pthread_join error:%s\n" ,strerror(err));
		else
		{
			printf("pthread [%x] join sucess\n",(unsigned int)tid[i]);
		}
	}
	//-------------------------------释放内存-----------------------------
	free(tid);
	tid = NULL;

	free(pCopy);
	pCopy = NULL;

	return 0;
}
