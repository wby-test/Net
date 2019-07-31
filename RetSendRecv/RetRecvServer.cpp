#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


#define     SERVERPORT 3000

int main(int ragc, char **argv)
{
    char buf[] = "helloworld\n";
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        std::cout << "create socket error  " << strerror(errno) << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port        = htons(SERVERPORT);

    if(bind(listenFd, (sockaddr *)&serverAddr,sizeof(serverAddr)) == -1)
    {
        std::cout << "bind error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    if(listen(listenFd, SOMAXCONN) == -1)
    {
        std::cout << "listen error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    sockaddr_in connAddr;
    socklen_t   connLen;
    int connFd;
    if((connFd = accept(listenFd, (sockaddr *)&connAddr, &connLen)) == -1)
    {
        std::cout << "accept error: " << strerror(errno) << std::endl;
        return -1;
    }

    int oldSockFlag = fcntl(connFd, F_GETFL, 0);
    int newSockFlag = oldSockFlag | O_NONBLOCK;
    if(fcntl(connFd, F_SETFL, 0) == -1)
    {
        std::cout << "set nonblockFd faild" << std::endl;
        return -1;
    }

    int count = 0;
    while(true)
    {  
        int iRetSend;
        int iRetRecv;

        iRetSend = send(connFd, buf, strlen(buf), 0);
        if(iRetSend == -1)
        {
            if(errno == EWOULDBLOCK)
            {
                std::cout << "socket is nonblock" << std::endl;
                continue;
            }
            else
            {
                std::cout << "send error: " << strerror(errno) << std::endl;
            }
            
            //break;
        }
        else if(iRetSend == 0)
        {
            std::cout << "peer closed connection" << std::endl;
            //break;
        }
        else
        {
            count++;
            std::cout << "send success count = " << count << std::endl;
        }
        
        // char recvBuf[32];
        // iRetRecv = recv(connFd, recvBuf, sizeof(recvBuf), 0);
        // if(iRetRecv == -1)
        // {
        //     std::cout << "recv error: " << strerror(errno) << std::endl;
        //     break;
        // }
        // else if(iRetRecv == 0)
        // {
        //     std::cout << "perr closed connection" << std::endl;
        //     //break;
        // }
        // else
        // {
        //     std::cout << "recv success: recvBuf is " << recvBuf << std::endl;
        // }
        
        //sleep(3);
    }

    close(connFd);
    close(listenFd);
    return 0;
}