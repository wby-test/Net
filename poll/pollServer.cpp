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
        std::cout  << "create socket FD error: " << strerror(errno)  << __LINE__ << std::endl;
        return -1;
    }

    //set listenfd to nonblock , when the fd is created;
    int oldSockFlags = fcntl(listenFd, F_GETFL, 0);
    int newSockFlags = oldSockFlags | O_NONBLOCK;
    if (fcntl(listenFd, F_SETFL, newSockFlags) == -1) {
        std::cout << "set the sock to nonblock error: " << strerror(errno)  << __LINE__ << std::endl;
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
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(3000);

    if (bind(listenFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cout << "bind error: " << strerror(errno)  << __LINE__ << std::endl;
        close(listenFd);
        return -1;
    }    

    if (listen(listenFd, SOMAXCONN) == -1) {
        std::cout << "listen error: " << strerror(errno)  << __LINE__ << std::endl;
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
                        int oldSockFlag = fcntl(clientFd, F_GETFL, 0);
                        int newSockFlag = oldSockFlag | O_NONBLOCK;
                        if (fcntl(clientFd, F_SETFL, newSockFlag) == -1) {
                            close(clientFd);
                            std::cout << "set nonblock clientFd error :" << strerror(errno) << std::endl;
                        }
                        else
                        {
                            pollfd client;
                            client.fd = clientFd;
                            client.events = POLLIN;
                            client.revents = 0;
                            fds.push_back(client);
                            std::cout << "add a new connect fd to fds: " << clientFd << std::endl;
                        }    
                    }
                }
                else
                {
                    char buf[64] = { 0 };
                    int iRetRecv = recv(fds[i].fd, buf, 64, 0);
                    if (iRetRecv <= 0) {
                        if(errno != EINTR || errno != EWOULDBLOCK) {
                            //error or perr closed connect
                            for (auto iter = fds.begin(); iter != fds.end(); iter++) {
                                if(fds[i].fd == iter->fd) {
                                    std::cout << "closed invalid connection " << iter->fd << std::endl;
                                    close(iter->fd);
                                    iter->fd = INVALID_FD;
                                    exist_invalid_fd = true;
                                    break;
                                }
                            }
                        }
                    }
                    else
                    {
                        std::cout << "recv data: " << buf << std::endl;
                    }     
                }
                
            }

            else if(fds[i].revents & POLLERR)
            {
                std::cout << "poll error" << std::endl;
            }
        }

        if(exist_invalid_fd) {
            for(auto iter = fds.begin(); iter != fds.end(); ) {
                if(iter->fd == INVALID_FD) {
                   iter = fds.erase(iter);
                }
                else {
                    ++iter;
                }
                
            }
        }

    }

    for(auto iter = fds.begin(); iter != fds.end(); ++iter) {
        close(iter->fd);
    }
    return 0;
}
