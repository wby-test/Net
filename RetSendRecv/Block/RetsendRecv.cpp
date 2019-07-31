#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>


#define SERVERPORT  3000
#define SEND_DATA   ""

int main(int argc, char **argv)
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        std::cout << "first: create socket fd error" << std::endl;
        return 0;
    }

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    serverAddr.sin_port         = htons(SERVERPORT);

    if(bind(listenFd, (sockaddr *)&serverAddr, sizeof(serverAddr) )== -1)
    {
        std::cout << "second: bind serveraddr error" << std::endl;
        return 0;
    }

    if(listen(listenFd, SOMAXCONN) == -1)
    {
        std::cout << "third: listen error" << std::endl;
        return -1;
    }

    sockaddr_in cliAddr;
    socklen_t   cliAddrLen = sizeof(cliAddr);
    int cliFd;
    if((cliFd = accept(listenFd, (sockaddr *)&cliAddr, &cliAddrLen)) == -1)
    {
        std::cout << "fourth: accept error" << std::endl;
        return -1;
    }

    while(true)
    {
        char buf[1024];
        int iRetSend;
        int iRetRecv;
        iRetSend = send(cliFd , SEND_DATA, strlen(SEND_DATA), 0);
        if(iRetSend == -1)
        {
            if(errno == EINTR)
            {
                std::cout << "fifth: signal EINTR" << std::endl;
            }
            else
            {
                std::cout << "fifth: send error" << std::endl;
                close(cliFd);
                close(listenFd);
                return -1;
            }
        }
        else if(iRetSend == 0)
        {
            std::cout << "block send = 0, send o byte, but ether not send" << std::endl;
            continue;
        } 
        else
        {
            std::cout << "send success, send: " << SEND_DATA << std::endl;
        }
        

        iRetRecv = recv(cliFd, buf, sizeof(buf), 0);
        if(iRetRecv == -1)
        {
            if(errno == EINTR)
            {
                std::cout << "recv: signal EINTR" << std::endl;
                continue;
            }
            else 
            {
                std::cout << "recv: send error" << std::endl;
                close(cliFd);
                close(listenFd);
                return -1;
            }
        }
        else if(iRetRecv == 0)
        {
            std::cout << "peer closed or recv 0 byte but not really recv" << std::endl;
        }
        else 
        {
            std::cout << "recv success, recv value is: " << buf << std::endl;
        }
    }

    close(listenFd);
    close(cliFd);


    return 0;
}


