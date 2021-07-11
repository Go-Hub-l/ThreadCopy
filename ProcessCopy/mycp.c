#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//错误码
enum ecode_err{
	argument_list_err,
	src_open_err,
	des_open_err,
	cop_fin
};

int main(int argc,char **argv)
{
	//错误处理
	if(argc < 3)
	{
		printf("请输入源文件和目标文件!\n");
		exit(argument_list_err);
	}
	int ProcessNum = 1;
	int nCount = 0;//标记第几个进程

	//字符转数字
	if(argc > 4)
	{
		ProcessNum = atoi(argv[3]);
		nCount = (int)*argv[4];
	}
	//文件描述符
	int sfd = open(argv[1],O_RDONLY);//源文件
	if(sfd < 0)
	{
		perror("open src error!\n");
		exit(src_open_err);
	}

	int dfd = open(argv[2],O_WRONLY | O_CREAT,0664);//目标文件
	if(dfd < 0)
	{
		perror("open des error!\n");
		exit(des_open_err);
	}

	//计算文件大小
	int fileSize = lseek(sfd,0,SEEK_END);

	//计算子进程需要拷贝的起点和大小
	int wNum = fileSize/ProcessNum;
	//文件可能不会被进程整除，因此不能除近时每个进程多拷贝一个
	wNum = fileSize%ProcessNum == 0 ? wNum : wNum+1;

	//读取数据缓冲区大小
	char buffer[8192] = {0};
	//读取的字节数和每个进程总共读取字节数标志
	int rsize;
	int flag = 0;

	//设置每个进程读取和写入的偏移指针位置
	int rseek = lseek(sfd,wNum*nCount,SEEK_SET);
	int wseek = lseek(dfd,wNum*nCount,SEEK_SET);

	while((rsize = read(sfd,buffer,sizeof(buffer)))!= 0)
	{
		flag += rsize;
		if(flag >= wNum)
		{
			//如果读取的数据大于要每个进程要写入的数据wNum
			rsize = wNum - (flag - rsize);
			write(dfd,buffer,rsize);
			//每一个进程拷贝结束后都要关闭自己的文件，描述符
			close(sfd);
			close(dfd);
			break;
			//exit(red_fin);
		}
		write(dfd,buffer,rsize);
	}
	//每一个进程拷贝结束后都要关闭自己的文件，描述符
	close(sfd);
	close(dfd);

	return cop_fin;
}
