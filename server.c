#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#define MAX sizeof(fd_set)*8
#define INIT -1
static void arrar_init(int arr_FD[],int num)
{
    int i=0;
    for(i=0;i<num;i++)
    {
        arr_FD[i]=INIT;
    }
}
static int arrar_add(int arr_FD[],int num,int fd)
{
    int i=0;
    for(i=0;i<num;i++)
    {
        if(arr_FD[i]==INIT)
        {
            arr_FD[i]=fd;
            return 0;
        }
    }
    return -1;
}
static void arrar_del(int arr_FD[],int num,int index)
{
    if(index<num&&index>0)
    {
        arr_FD[index]=INIT;
    }
}

int set_rfds(int arr_FD[],int num,fd_set*rfds)
{
    int i=0;
    int max_fd=INIT;
    for(i=0;i<num;i++)
    {
        if(arr_FD[i]>INIT)
        {
            FD_SET(arr_FD[i],rfds);

            if(max_fd<arr_FD[i])
            {
                max_fd=arr_FD[i];
            }

        }
    }
    return max_fd;
}

void service(int arr_FD[],int num,fd_set* rfds)
{
    int i=0;
    for(i=0;i<num;i++)
    {
        if(arr_FD[i]>INIT&&FD_ISSET(arr_FD[i],rfds))
        {
            int fd=arr_FD[i];
            if(i==0)//表示是监听套接字
            {
                struct sockaddr_in client;
                socklen_t len=sizeof(client);
                int new_sock=accept(fd,(struct sockaddr*)&client,&len);
                if(new_sock<0)
                {
                    perror("accept");
                    continue;
                }
                printf("get a new connection [%s %d]\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                if(arrar_add(arr_FD,num,new_sock)<0)
                {
                    printf("server is busy!\n");
                    close(new_sock);
                }
            }
            else//表示是常规文件描述符
            {
                char buf[1024]={0};
                ssize_t s=read(fd,&buf,sizeof(buf)-1);
                if(s>0)
                {
                    buf[s]=0;
                    printf("client say# %s\n",buf);
                }
                else if(s==0)
                {
                    close(fd);
                    printf("client quit\n");
                    arrar_del(arr_FD,num,i);
                }
                else
                {
                    perror("read");
                    close(fd);
                    arrar_del(arr_FD,num,i);
                }
            }
        }
    }
}
int StartUp(int port)
{
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }

    int opt=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(port);
    local.sin_addr.s_addr=htonl(INADDR_ANY);
    
    if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
    {
        perror("bind");
        exit(2);
    }
    if(listen(sock,5)<0)
    {
        perror("listen");
        exit(3);
    }
    return sock;
}
int main(int argc,char* argv[])
{
    //./server 9999
    if(argc!=2)
    {
        printf("Usage:{%s port}\n",argv[0]);
        return 1;
    }
    int listen_sock=StartUp(atoi(argv[1]));
    int arr_FD[MAX];//这里需要一个数组来进行每次重新设置位图中的文件描述符
    arrar_init(arr_FD,MAX);//对位图的初始化
    arrar_add(arr_FD,MAX,listen_sock);//添加至fd_set
    fd_set rfds;
    int max_fd=0;
    for(;;)
    {
        struct timeval timeout={5,0};
        FD_ZERO(&rfds);
        max_fd = set_rfds(arr_FD,MAX,&rfds);
        switch(select(max_fd+1,&rfds,NULL,NULL,&timeout))//只关心读时间
        {
            case 0:
                printf("select timeout.....!\n");
                break;
            case -1:
                perror("select");
                break;
            default:
                service(arr_FD,MAX,&rfds);
                break;
        }
    }
}

