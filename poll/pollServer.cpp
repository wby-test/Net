#include <iostream>
#include <string.h>
#include <vector>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

#define INVALID_FD  -1


int main(int argc, char **argv)
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd == -1) {
        std::cout  << "create socket FD error: " << strerror(errno) << std::endl;
        return -1;
    }

    //set listenfd to nonblock , when the fd is created;
    int oldSockFlags = fcntl(listenFd, F_GETFL, 0);
    int newSockFlags = oldSockFlags | O_NONBLOCK;
    if (fcntl(listenFd, newSockFlags, 0) == -1) {
        std::cout << "set the sock to nonblock error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    //reuse addr and port; 
    //wby--: why not checkout return;
    int on = 1;
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    setsockopt(listenFd, SOL_SOCKET, SO_REUSEPORT, (char *)&on, sizeof(on));

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    serverAddr.sin_port = htons(3000);

    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cout << "bind error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }    

    if (listen(listenFd, SOMAXCONN) == -1) {
        std::cout << "listen error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    std::vector<pollfd> fds;
    pollfd listen_fd_info;
    listen_fd_info.fd = listenFd;
    listen_fd_info.events = POLLIN;
    listen_fd_info.revents = 0;
    fds.push_back(listen_fd_info);

    bool exist_invalid_fd = false;
    int iRet;

    while(true) {
        iRet = poll(&fds[0], fds.size(), 1000);
        if (iRet < 0) {
            if (errno == EINTR) {
                std::cout << "EINTR" << std::endl;
                continue;
            }
            break;
        }

        if (iRet == 0) {
            std::cout << "timeout: " << std::endl;
            continue;
        }

        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == listenFd) {
                    struct sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);

                    int clientFd = accept(fds[i].fd, (struct sockaddr*)&clientAddr, &clientLen);
                    if(clientFd != -1) {
                        int oldSockFlag = fcntl(clientFd, GET)
                    }
                }
            }
        }

    }

    return 0;
}