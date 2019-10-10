#include <iostream>
#include <string.h>
#include <errno.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <poll.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1)
    {
        std::cout << "create socket error: " << strerror(errno) << std::endl;
        return -1;
    }

    int on = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));

    int oldSockFlag = fcntl(listenFd, F_GETFL, 0);
    int newSockFlag = oldSockFlag | newSockFlag;
    if(fcntl(listenFd, F_SETFL, newSockFlag) == -1)
    {
        std::cout << "set nonblock sock error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port        = htons(3000);

    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cout << "bind error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    if (listen(listenFd, SOMAXCONN) == -1)
    {
        std::cout << "listen error: "  << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    //epoll 
    int epollFd = epoll_create(1);
    if (epollFd == -1)
    {
        std::cout << "create epoll fd error: " << strerror(epollFd) << std::endl;
        close(listenFd);
        return -1;
    }

    epoll_event event;
    event.data.fd = listenFd;
    event.events = EPOLLIN;

    //边缘模式（event.events |= EPOLLEF） 和 水平模式
    //event.events |= EPOLLET;

    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, listenFd, &event) == -1)
    {
        std::cout << "EPOLL_CTL_ADD error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    int n;
    while (true)
    {
        epoll_event epoll_events[1024];
        n = epoll_wait(epollFd, epoll_events, 1024, 1000);
        if (n < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            break;
        }
        else if (n == 0)
        {
            //超时
            continue;
        }
        for (size_t i = 0; i < n; ++i)
        {
            if (epoll_events[i].events & EPOLLIN)
            {
                if (epoll_events[i].data.fd == listenFd)
                {
                    struct sockaddr_in clientAddr;
                    socklen_t sockLen = sizeof(clientAddr);

                    int clientFd = accept(listenFd, (struct sockaddr *)&clientAddr, &sockLen);
                    if (clientFd != -1)
                    {
                        int oldSockFlagCli = fcntl(clientFd, F_GETFL, 0);
                        int newSockFlagCli = oldSockFlagCli | O_NONBLOCK;
                        if (fcntl(clientFd, F_SETFL, newSockFlagCli) == -1)
                        {
                            close(clientFd);
                            std::cout << "set clisock nonblock error: " << strerror(errno) << std::endl;
                        }
                        else
                        {
                            epoll_event cliEvent;
                            cliEvent.data.fd = clientFd;
                            cliEvent.events  = EPOLLIN;
                            //边缘模式
                            cliEvent.events |= EPOLLET;
                            epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &cliEvent);
                        }
                        
                    }
                    
                }
            }
        }
    }

    return 0;
}