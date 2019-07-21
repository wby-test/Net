#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/times.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>



int main(int argc, char **argv)
{
    int i, maxi, maxfd, listenfd, connfd, sockfd;
    int nready, client[256];
    ssize_t n;
    fd_set rSet, allSet;
    char buf[1024];
    socklen_t clientLen;
    struct sockaddr_in clientAddr, serverAddr;

    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("listen socket error %s", strerror(errno));
        return -1;
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(atoi(argv[1]));

    if(bind(listenfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("bind error %s", strerror(errno));
        return -1;
    }

    if((listen(listenfd, 256)) < 0)
    {
        printf("listen error %s", strerror(errno));
        return -1;
    }
    
    maxfd = listenfd;
    maxi = -1;
    for(i = 0; i < 256; i++)
    {
        client[i] = -1;
    }

    FD_ZERO(&allSet);
    FD_SET(listenfd, &allSet);

    for(; ;)
    {
        rSet = allSet;
        if ((nready = select(maxfd + 1, &rSet, NULL,  NULL, NULL)) < 0)
        {
            printf("select error %s", strerror(errno));
            return -1;
        }

        if(FD_ISSET(listenfd, &rSet))
        {
            clientLen = sizeof(clientAddr);
            if((connfd = accept(listenfd, (struct sockaddr *)&clientAddr, &clientLen)) < 0)
            {
                printf("accept error %s %d", strerror(errno), clientLen);
                return -1;
            } 
            for(i = 0; i < 256; i++)
            {
                if(client[i] < 0)
                {
                    client[i] = connfd;
                    break;
                }
            }

            if(i == 256)
            {
                printf("too many client");
                return -1;
            }

            FD_SET(connfd, &allSet);

            if(connfd > maxfd)
            {
                maxfd = connfd;
            }

            if(i > maxi)
            {
                maxi = i;
            }

            if(--nready <= 0)
            {
                continue;
            }
        }

        for(i = 0; i <= maxi; i++)
        {
            if( (sockfd = client[i]) < 0 )
            {
                continue;
            }

            if(FD_ISSET(sockfd, &rSet))
            {
                if((n = read(sockfd, buf, 1024)) == 0)
                {
                    printf("buf is : %s", buf);
                    /*connection closed by client*/
                    close(sockfd);
                    FD_CLR(sockfd, &allSet);
                    client[i] = -1;
                }  else
                {
                    write(sockfd, buf, n);
                }
            
                if(--nready <= 0)
                {
                    break;
                }
            }
        }
    }   
}



/****************************************************************************
 * when client shutdown, the socket will get data;
 * find why!!!!!!!!!!!!!!! the stream function have questions;
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 *
*******************************************************************************/