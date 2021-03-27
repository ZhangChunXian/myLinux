	/*************************************************
	    目的：
		１、应用UNIX的fork()等系统调用，编写一个c程序具有以下功能：
		  a) 实现Shell的基本功能，包括有：打印提示符；
		      接受和分析命令行（滤去无效的空格、tab符号以及换行符等）；
		      执行命令（要有出错处理；输入exit或者bye退出）；返回父进程；
		  b) 处理后台程序（不需要wait)
		  c) 处理多行命令（分析命令行中的‘;’并处理之）
		  d)应用 dup(), pipe()系统调用具有输入输出重定向以及管道功能；

	    文件名：my_shell.c
	    作者：鹿珂珂 2005011534
	    日期：2008年5月31日
	***************************************************/
	

	/************头文件************/
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <string.h>
	#include <fcntl.h>
	#include <unistd.h>



  	/**************全局变量定义***************/
	const int BUFFERSIZE=80;//接收命令的最大字符数
	char buffer[80];//缓存用户输入的命令

	int is_back=0;//是否是后台进程,默认不是后台执行的程序
	int status=0;//状态变量
	pid_t pid;//进程


  	
	/**************函数声明***************/
	char *get_command(int *input_len);//获取输入命令
	void dispose_command(char *deal_in,int len);//解析处理命令
	int redirect(char * in, int len);//实现输入输出重定向的函数
	int piple(char * in, int li_inlen);//实现管道功能的函数
	int is_fileexist(char * comm);//用来查找命令是否存在
	void multi_command(char * m_in, int m_len);//处理用分号区分的多个命令
	

	int main()//主函数：调用get_command()和dispose_command()实现shell的基本功能
	{
		char * path;//当前路径
		char *user;//用户名
		char *input=NULL;//用户输入命令
		int command_len=0;//输入字符个数
		while(1) 
		{
			command_len=0;
			user=getlogin();//获取当前登录用户
			printf("%s@%s-desktop:~$",user,user);//用户输入提示符：用户名＠用户名-desktop：～当前目录$
			if(input)
				free(input);//释放上一次输入命令内存空间
			input=get_command(&command_len);//获取命令
			if(input)
			{
				dispose_command(input,command_len);//处理执行命令
			}
		}
	}



	char *get_command(int * input_len)//获取用户输入命令
	{
		char lc_char;//输入字符
		char *get_in;
		(*input_len)=0;	
		/* 开始获取输入 */
		lc_char = getchar();
		while(lc_char != '\n' && (*input_len) < BUFFERSIZE)
		{
	    		buffer[(*input_len) ++] = lc_char;
	   	 	lc_char = getchar();
		}

		/* 命令超长处理*/
		if((*input_len) >= BUFFERSIZE) {
	  	   printf("Your command too long ! Please reenter your command !\n");
	  	   (*input_len) = 0;     /* Reset */
		   gets(buffer);
		   strcpy(buffer,"");
		   get_in=NULL;
	    	   return NULL;
		}
		else  
	   		buffer[(*input_len)] = '\0'; /*加上串结束符，形成字符串*/

		if((*input_len)==0)return NULL;//处理直接敲入回车的情况

		/* 将命令从缓存拷贝到in中*/
		get_in = (char *)malloc(sizeof(char) * ((*input_len) + 1));
		strcpy(get_in, buffer);
		strcpy(buffer,"");
		return get_in;
	}


	void dispose_command(char *deal_in,int len)//调用multi_command()、piple（）、redirect()分别实现多个命令、管道、重定向
	{
		char * arg[30];//存储指令或参数
		int i=0, j=0, k=0;//计数变量
		pid_t pid;//进程
		/* 获取命令和参数并保存在arg中*/
		 for( i=0;i<=len;i++)
		 {
			if(deal_in[i]==';')
			{
				multi_command(deal_in,len);
				return;
			}
		 }
		 for( i = 0, j=0,k=0; i<=len; i ++) {
			/*管道和重定向单独处理*/
			if(deal_in[i]=='<' || deal_in[i] == '>' || deal_in[i] == '|' || deal_in[i]==';') {
			  if(deal_in[i] == '|')
				{
			      	     piple(deal_in, len);
				     return;
				}
			   else if(deal_in[i]=='>' ||deal_in[i]=='<')
			       {
			       	     redirect(deal_in, len);
				     return;
				}
			}
			/*处理空格，TAB和结束符。不用处理'\n'，大家如果仔细分析前面的获取输入的程序的话，不难发现回车符并没有写入buffer*/
			if(deal_in[i]== ' ' || deal_in[i] == '\t' || deal_in[i] == '\0') {
			   	if(j == 0) /*这个条件可以略去连在一起的多个空格或者TAB */
			     	 continue;
			   	else {

				       buffer[j ++] = '\0';    //结束上一个字符串
				       arg[k] = (char *)malloc(sizeof(char) * j);//将第一个字符串拷贝至arg

				       /* 将指令或参数从缓存拷贝到arg中*/
				       strcpy(arg[k], buffer);
			      	       j = 0;  /* 准备取下一个参数*/
			      	       k ++;
			   	     }
			}
			else {       /* 如果字符串最后是 '&'，则置后台运行标记为 1 */
			  	  if(deal_in[i]== '&' && deal_in[i + 1] == '\0') {
					is_back = 1;//是后台进程

					return;//后台进程则返回，继续接收指令
			     }

				buffer[j ++] = deal_in[i];
			}
		}
		if(!strcmp(arg[0], "bye") || !strcmp(arg[0],"exit")) {   //如果输入 bye或者exit 则退出程序
			printf("bye-bye\n");
			exit(0);
		}

		/* 在使用exec执行命令的时候，最后的参数必须是NULL指针，所以将最后一个参数置成空值*/
		arg[k] = (char *)malloc(sizeof(char));
		arg[k ++] = (char *)0;
		/* 判断指令arg[0]是否存在*/
		if(is_fileexist(arg[0]) == -1) {
		   	 printf("This is command is not founded ?!\n");
		   	 for(i = 0; i < k; i ++)
			 free(arg[i]);
		   	 return;

		}
		if((pid = fork()) == 0)      /*子进程*/
		     execv(buffer, arg);
		else                     /*父进程*/
		   if(is_back == 0)        /* 并非后台执行指令*/
		       waitpid(pid, &status, 0);

		   /* 释放申请的空间*/
		   for(i = 0; i < k; i ++)   free(arg[i]);
	}

	int redirect(char * in, int len)//实现重定向功能
	{

		char * argv[30], *filename[2];

		int fd_in, fd_out, is_in = -1, is_out = -1, num = 0;
		int is_back=0;
		int I,i, j, k=0;//计数变量
		int status=0;


		/* 这里是重定向的命令解吸过程，其中filename用于存放重定向文件，is_in, is_out分别是输入重定向标记和输出重定向标记*/
		for(I = 0, j = 0, k = 0; I <= len; I ++) {
  			if(in[I] == ' ' || in[I] == '\t' || in [I] == '\0' || in[I] == '<' || in[I] == '>') {
        			if(in[I] == '>' || in[I] == '<') {
        			/* 重定向指令最多 '<', '>'各出现一次，因此上num最大为2，否则认为命令输入错误*/
       			 		if(num < 3) {
           					num ++;
          					if(in[I] == '<') 
               						is_in = num - 1;
           					else  is_out = num - 1;
        					/* 处理命令和重定向符号相连的情况，比如 ls > a*/
        					if(j > 0 &&num == 1) {
          						buffer[j ++] = '\0';
              						argv[k] = (char *)malloc(sizeof(char) * j);
              						strcpy(argv[k], buffer);
             						k ++;
             						j = 0;
        					}
   					}
     					else {
          					printf("The format is error !\n");
           					return - 1;
                			}
              			}
		        	if(j == 0) 
		             		continue;
		        	else {
					buffer[j ++] = '\0';
					/* 尚未遇到重定向符号，字符串时命令或参数*/
					if(num == 0){
	    					argv[k] = (char *)malloc(sizeof(char) * j);
	   					strcpy(argv[k], buffer);
	   					k ++;
					}
					/* 是重定向后符号的字符串，是文件名*/
					else {
	  					filename[status] = (char *)malloc(sizeof(char) * j);
	  					strcpy(filename[status], buffer);
					}
					j = 0;   /* Initiate */ 
					continue;
		           	}
               		}
                	else{
                   		if(in[I] == '&' && in[I + 1] == '\0') {
				is_back = 1;
				return;
				}
				buffer[j ++] = in[I];
               		}
		}
		argv[k] = (char *)malloc(sizeof(char));
		argv[k ++] = (char *)0;

		if(is_fileexist(argv[0]) == -1) {
      			printf("This command is not founded !\n");
       			for(I = 0; I < k; I ++) free(argv[I]);
      			return 0;
		}
		
		if((pid = fork()) == 0) {
     			/* 存在输出重定向*/
      			if(is_out != -1) {
          			if((fd_out = open(filename[is_out], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR )) == -1) 				{
           				printf("Open out %s error \n", filename[is_out]);
           				return -1;
      				}
  			}
			/* 存在输入重定向*/
			if(is_in != -1) {
     				if((fd_in = open(filename[is_in], O_RDONLY, S_IRUSR|S_IWUSR)) == -1) {
  					printf("Open in %s error \n", filename[is_in]);
  					return -1;
				}
			}
			if(is_out != -1) {
				/* 使用dup2函数将标准输出重定向到fd_out上，dup2(int oldfd, int  newfd)
				实现的是报oldfd所指的文件描述符复制到newfd。若newfd为已已打开的文件描述符，
				则newfd所指的文件先会被关闭，dup2复制的文件描述符与原来的文件描述符共享各种文件状态*/
				if(dup2(fd_out, STDOUT_FILENO) == -1) {
	     				printf("Redirect Standard Out Error !\n");
	     				exit(1);
	  			}
			}
			if(is_in != -1) {
	    			if(dup2(fd_in, STDIN_FILENO) == -1) {
	       				printf("Redirect Standard In error !\n");
	       				exit(1);
	    			}
			}
			execv(buffer, argv);
          	}
		else 
                 	if(is_back == 0)        /* Run on the TOP */
				waitpid(pid, &status, 0);
		for(I = 0; I < k; I ++) 
			free(argv[I]);
          	if(is_in != -1) {
                	free(filename[is_in]);
                	close(fd_in);
           	}
          	if(is_out != -1){
	            	free(filename[is_out]);
                	close(fd_out);
         	 }
          	return 0;
	}


	void multi_command(char * m_in, int m_len)//处理用分号区分的多个命令
	{
		int pos;  //记录分号位置
		int i=0,j=0;
		char *command=NULL;  //拆分存储命令
		for(i=0;i<=m_len;i++)
		{
			if(m_in[i]==';')     //处理分号前的第一个命令
			{
				pos=i;
				command=(char *)malloc(sizeof(char) * (i+1));
				for(j=0;j<i;j++)
					command[j]=m_in[j];
					command[j]='\0';
					dispose_command(command,i+1);
					//printf("%s  %d  %d\n",command,m_len,pos);
					free(command);  //释放动态开辟的存储空间
					break;
			}
		}
		if(m_in[pos+1]=='\0')   //如果以分号结尾，则退出循环
			return;
		if(pos>=m_len)return;      //与上面一句相同作用
		command=(char *)malloc(sizeof(char) * (m_len-pos));  //处理第一个分号以后的命令
		for(i=pos+1,j=0;i<=m_len;i++)
		{
			command[j++]=m_in[i];
		}
		dispose_command(command,m_len-pos);    //递归调用处理剩余命令
		free(command);
	}

	int piple(char * in, int li_inlen)//实现管道功能
	{
	   char * argv[2][30];
	   int  count;
	   is_back = 0;
	   int li_comm = 0, fd[2], fpip[2];
	   char lc_char=' ';
	   char lc_end[1];
	   pid_t child1, child2;
	   int I,i, j, k=0;//计数变量
	   /* 管道的命令解析过程*/
	   for(I = 0, j = 0, k = 0; I<=li_inlen; I ++) {
		      if(in[I]== ' ' || in[I] == '\t' ||in[I] == '\0' || in[I] == '|') {

				  if(in[I] == '|') {          /* 管道符号*/

					      if(j > 0) {
						   buffer[j ++] = '\0';

					/* 因为管道时连接的是两个指令，所以用二维数组指针来存放命令和参数， li_comm是表示的几个指令*/
						   argv[li_comm][k] = (char *) malloc(sizeof(char) * j);
						   strcpy(argv[li_comm][k++], buffer);
					      }
					  //    argv[li_comm][k] = (char *)malloc(sizeof(char));
				      	      argv[li_comm][k++] = (char *)0;

				 	     /* 遇到管道符，第一个指令完毕，开始准备第二个指令*/
				   	     li_comm++;
				    	     count = k;
					     k = 0; j = 0;
				   }

				   if(j == 0)  continue;

				   else {
					       buffer[j ++] = '\0';
					       argv[li_comm][k] = (char *)malloc(sizeof(char) * j);
					       strcpy(argv[li_comm][k], buffer);
					       k ++;
				    }

				    j = 0; /* Initiate */
				   continue;
			 }

			 else {
			     	if(in[I] == '&' && in[I + 1] == '\0') {
					 is_back = 1; //是后台进程则返回

					 return 0;    //后台进程不进行处理，直接返回
			        }
		     	 }
		         buffer[j ++] = in[I];
	   }
		//argv [li_comm][k] = (char *)malloc(sizeof(char));
   		argv [li_comm][k++] = (char *)0;

	    if(is_fileexist(argv[0][0]) == -1) {
		 printf("This first command is not founed !\n");
		 for(I = 0; I < count; I ++) free(argv[0][I]);
		 return 0;
	    }

	    /* 指令解析结束*/
	    /* 建立管道*/
	    if(pipe(fd) == -1) {
		printf("Open pipe error !\n");
		return -1;
		 }

		  /*创建的一个子进程执行管道符前的命令，并将输出写道管道*/
		  if((child1 = fork()) == 0) {
			/*关闭读端*/
			close(fd[0]);
			if(fd[1] != STDOUT_FILENO) {
			    /* 将标准的输出重定向到管道的写入端，这样该子进程的输出就写入了管道*/
				 if(dup2(fd[1], STDOUT_FILENO) == -1) {
				      printf("Redirect Standard Out error !\n");
				      return -1;
				 }
				/*关闭写入端*/
				close(fd[1]);
			}
			execv(buffer, argv[0]);
		  }
		  else { 					/*父进程*/
			/* 先要等待写入管道的进程结束*/
			waitpid(child1, &li_comm,0);
			/* 然后我么必须写入一个结束标记，告诉读管道进程的数据到这里就完了*/
			lc_end[0] = 0x1a;
			write(fd[1], lc_end, 1);
			close(fd[1]);
			if(is_fileexist(argv[1][0]) == -1) {
			      printf("This command is not founded !\n");
			      for(I = 0; I < k; I ++) free(argv[1][I]);
			      return 0;
			 }

			/* 创建第二个进程执行管道符后的命令，并从管道读书入流*/
			if((child2 = fork()) == 0) {
			     if(fd[0] != STDIN_FILENO) {
				/* 将标准的输入重定向到管道的读入端*/
				if(dup2(fd[0], STDIN_FILENO) == -1) {
				    printf("Redirect Standard In Error !\n");
				    return -1;
				}
				close(fd[0]);
			     }
			     execv(buffer, argv[1]);
			  }
			  else      /*父进程*/
			      if(is_back == 0)  waitpid(child2, NULL, 0);
			}
			for(I = 0; I < count; I ++)  free(argv[0][I]);
			for(I = 0; I < k ; I ++) free(argv[1][I]);
			return 0;	
	}


	int is_fileexist(char * comm)//查找命令是否存在
	{
		char * env_path, * p;
		int i=0;
		/* 使用getenv函数来获取系统环境变量，用参数PATH表示获取路径*/
		env_path = getenv("PATH");
		p = env_path;
		while(*p != '\0') {
		   /* 路径列表使用"："来分隔路径*/
		   if(*p != ':')  buffer[i ++] = *p;
		   else {
		       buffer[i ++] = '/';
		       buffer[i] = '\0';
		       /* 将指令和路径合成，形成pathname，并使用access函数来判断该文件是否存在*/
		       strcat(buffer, comm);
		       if(access(buffer, F_OK) == 0)     /* 文件被找到*/
			     return 0;
		       else                          /* 继续寻找其他路径*/
			     i = 0;
		    }
		    p ++;
		 }
		 /* 搜索完所有路径，依然没有找到则返回 –1*/
		 return -1;
	}