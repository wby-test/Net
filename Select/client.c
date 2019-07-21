#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/times.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

void str_cli(FILE *fp, int sockfd)
{
    char sendLine[10], recieveLine[10];
    while(fgets(sendLine, 1024, fp) != NULL)
    {
        write(sockfd, sendLine, 1024);
        if(read(sockfd, recieveLine, 1024) == 0)
        {
            printf("server closed prematurely");
            return;
        }
        fputs(recieveLine, stdout);
    }
}

int main(int argc, char **argv)
{
    int sockefd;
    struct sockaddr_in serverAddr;
    
    
    if(argc < 3)
    {
        printf("arguments not enough %d", argc);
        return -1;
    }
    if((sockefd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("socket error %s", strerror(errno));
        return -1;
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[2]));
    if((inet_pton(AF_INET, argv[1], &serverAddr.sin_addr)) <= 0)
    {
        printf("inet_pton error %s", strerror(errno));
        return -1;
    }

    if((connect(sockefd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0)
    {
        printf("connect error %s", strerror(errno));
        return -1;
    }

    str_cli(stdin, sockefd);
    return 0;
}