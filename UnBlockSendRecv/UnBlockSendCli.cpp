#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define  SERVERPORT 3000
#define  SERVERADDR "127.0.0.1"
#define  SENDDATA   "helloworld"

int main(int argc, char **argv)
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientFd == -1)
    {
        std::cout << "create socket error: " << strerror(errno) << std::endl;
        return -1;
    }

    sockaddr_in servAddr;
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(SERVERADDR);
    servAddr.sin_port        = htons(SERVERPORT);

    int ret = connect(clientFd, (sockaddr *)&servAddr, sizeof(servAddr));
    if(ret == -1)
    {
        std::cout << "connect server error: " << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    int oldFdFlags = fcntl(clientFd, F_GETFL, 0);
    int newFdFlags = oldFdFlags | O_NONBLOCK;
    if(fcntl(clientFd, F_SETFL, newFdFlags) == -1)
    {
        std::cout << "set nonblock socket error: " << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    int count = 0;

    while(true)
    {
        int sendRet = send(clientFd, SENDDATA, strlen(SENDDATA), 0);
        if(sendRet == -1)
        {
            if(errno == EWOULDBLOCK)
            {
                std::cout << "unblocksocket" << std::endl;
                continue;
            }
            else if(errno == EINTR)
            {
                std::cout << "io was interate by inter" << std::endl;
                continue;
            }
            else
            {
                std::cout << "send error: " << strerror(errno) << std::endl;
                break;
            }
            
        }
        else if(sendRet == 0)
        {
            std::cout << "connection was closed by peer" << std::endl;
            break;
        }
        else
        {
            count++;
            std::cout << "send data success " << count << std::endl;
        }
    }

    close(clientFd);
    return 0;
}
