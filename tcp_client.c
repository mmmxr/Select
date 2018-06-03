#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main(int argc,char* argv[])
{
    if(argc!=3)
    {
        printf("Usage:%s:ip port\n",argv[0]);
        exit(1);
    }
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        printf("sock error\n");
        exit(2);
    }
    struct sockaddr_in server;
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=inet_addr(argv[1]);
    server.sin_port=htons(atoi(argv[2]));

    if(connect(sock,(struct sockaddr*)&server,sizeof(server))<0)
    {
        printf("connect error!\n");
        exit(3);
    }
    char buf[64];
    while(1)
    {
        printf("please enter# ");
        fflush(stdout);
        size_t s=read(0,&buf,sizeof(buf));
        if(s>0)
        {
            buf[s-1]=0;
            if(strcmp("quit",buf)==0)
            {
                printf("client quit!\n");
                break;
            }
        }
        write(sock,&buf,sizeof(buf));
    }
    close(sock);
    return 0;
}
