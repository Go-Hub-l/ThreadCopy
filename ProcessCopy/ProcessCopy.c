#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

//错误码
enum ecode_err{
	argument_list_err = 3,
	argument_many,
	ProcessNum_err,
	fork_err,
};

int main(int argc,char **argv)
{
	//校验传入参数
	if(argc < 3)
	{
		printf("请输入要拷贝的源文件和目标文件!(进程数可选择是否输入：0~100)\n");
		exit(argument_list_err);
	}
	if(argc > 4)
	{
		printf("最多传三个参数!\n");
		exit(argument_many);
	}

	int ProcessNum = 5;//默认创建5个进程

	if(argc == 4)
	{
		ProcessNum = atoi(argv[3]);
	}

	//校验用户输入的线程数
	if(ProcessNum <= 0 || ProcessNum >= 100)
	{
		printf("请输入0～100之间内的进程数\n");
		exit(ProcessNum_err);
	}
	/*-----------------------------------开始拷贝------------------------------------*/
	//创建子进程
	pid_t pid;
	int i;
	for(i = 0;i < ProcessNum;i++)
	{
		pid = fork();
		//子进程跳出循环，只有父进程来创建进程
		if(pid == 0)
			break;
	}
	if(pid < 0)	
	{
		perror("parent call fork fail...\n");
		exit(fork_err);
	}
	/*---------子进程拷贝--------------*/
	if(i < ProcessNum)//子进程
	{
		char *argv1[] = {"copy",argv[1],argv[2],argv[3],(char*)&i,NULL};
		int ecode = execv("copy",argv1);
	}

	/*-----------------------------回收子进程空间------------------------------------------*/
	pid_t wpid;
	int status;
	int ecode;
	while((wpid = waitpid(-1,&status,WNOHANG)) != -1)
	{
		if(wpid > 0)
		{
			//子进程正常退出
			if(WIFEXITED(status))
			{
				ecode = WEXITSTATUS(status);
				printf("child wait success pid = %d  ecode = %d\n",wpid,ecode);
			}
			else if(WIFSIGNALED(status))//信号杀死
			{
				ecode = WTERMSIG(status);
				printf("child waip sig pid = %d   ecode = %d\n",wpid,ecode);
			}
		}
	}

	while(1);

	return 0;
}
