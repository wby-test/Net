#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT    3000

int main(int argc, char **argv)
{
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientFd == -1)
    {
        std::cout << "create socket error " << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serverAddr.sin_port        = htons(SERVER_PORT);

    if(connect(clientFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cout << "connect error " << std::endl;
        std::cout << strerror(errno) << std::endl;
        close(clientFd);
        return -1;
    }

    fd_set readSet;
    FD_ZERO(&readSet);

    FD_SET(clientFd, &readSet);
    timeval tm;
    tm.tv_sec  = 5;
    tm.tv_usec = 0;

    int ret;
    int count = 0;
    fd_set backup_readset;
    memcpy(&backup_readset, &readSet, sizeof(fd_set));
    std::cout << "begin while" << std::endl;
    while(true)
    {
        if(tm.tv_sec == 0)
        {
            std::cout << "why" << std::endl;
            break;
        }
        if(memcmp(&readSet, &backup_readset, sizeof(fd_set)) == 0)
        {
            std::cout << "readset = backup_readset" << std::endl;
        }
        else
        {
            std::cout << "readset != backup_readset" << std::endl;
        }

        ret = select(clientFd + 1, &readSet, NULL, NULL, &tm);
        std::cout << "tm.tv_sec: " << tm.tv_sec << " tm.tv_usec: " << tm.tv_usec << std::endl;
        if(ret == -1)
        {
            if(errno != EINTR)
            {
                std::cout << strerror(errno) << std::endl;
                break;
            }
        }
        else if (ret == 0)
        {
            std::cout << strerror(errno) << std::endl;
            std::cout << "no event in specific time interval, cout: " << count << std::endl;
            ++count;
            continue;
        }
        else
        {
            if(FD_ISSET(clientFd, &readSet))
            {
                char recvBuf[32];
                memset(recvBuf, 0, sizeof(recvBuf));
                int n = recv(clientFd, recvBuf, 32, 0);
                if(n < 0)
                {
                    std::cout << strerror(errno) << std::endl;
                    if(errno != EINTR)
                        break;
                }
                else if(ret == 0)
                {
                    std::cout << "peer close connection" << std::endl;
                    break;
                }
                else
                {
                    std::cout << "recv data: " << recvBuf << std::endl;
                }
            }
            else
            {
                std::cout << "other socket event." << std::endl;
            }
        }   
    }
    close(clientFd);
    return 0;
}