#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(int argc, char **argv)
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientFd == -1)
    {
        std::cout << "create sockfd error" << std::endl;
        return -1;
    }

    int oldSockFlag = fcntl(clientFd, F_GETFL, 0);
    int newSockFlag = oldSockFlag | O_NONBLOCK;
    if(fcntl(clientFd, F_SETFL, newSockFlag) == -1)
    {
        close(clientFd);
        std::cout << "set nonblock socket error" << std::endl;
        return -1;
    }

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("192.168.3.12");
    serverAddr.sin_port        = htons(3000);

    for(;;)
    {
        int iRet = connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
        if(iRet == 0)
        {
            std::cout << "connet to remote server successfuly" << std::endl;
            send(clientFd, "connect", strlen("connect"), 0);
            break;
        }
        else if(iRet == -1)
        {
            if(errno == EINPROGRESS)
            {
                std::cout << "UNBLOCK SOCKET" << std::endl;
                break;
            }
            else if(errno == EINTR)
            {
                std::cout << "connection was interuptted by signal, try again" << std::endl;
                continue;
            }
            else
            {
                std::cout << "connection error" << std::endl;
                close(clientFd);
                return -1;
            }   
        }
    }
    
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(clientFd, &writeset);

    struct timeval tv;
    tv.tv_sec  = 3;
    tv.tv_usec = 0;

/******************************************************************************************
 *on linux, when socket has not connected, select return can write, 
 *on windows, return can not write; 
 *so, on linux, select has to check is not error
 *eg:getsockopt()
 * ****************************************************************************************/
    if(select(clientFd + 1, NULL, &writeset, NULL, &tv) != 1)
    {
        std::cout << "[select] connect to remote server failed" << std::endl;
        close(clientFd);
        return -1;
    }

    int iRetErr;
    //w-- static_case<>();
    socklen_t len = static_cast<socklen_t>(sizeof(iRetErr));
     if(::getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &iRetErr,&len) < 0)
     {
         close(clientFd);
         return -1;
     }
     if(iRetErr == 0)
     {
         std::cout << "connect to remoter server successflly" << std::endl;
     }
     else
     {
         std::cout << "connect to remote server failed" << std::endl;
     }
     
    close(clientFd);
    return 0;
}