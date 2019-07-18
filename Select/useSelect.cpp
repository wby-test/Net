#include <sys/socket.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <vector>


#define INVALID_FD  -1

int main()
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        std::cout << "create listen sock error;" << std::endl;
        return -1;
    }

    sockaddr_in connFd;
    bzero(&connFd, sizeof(connFd));
    connFd.sin_family       = AF_INET;
    connFd.sin_addr.s_addr  = htonl(INADDR_ANY);
    connFd.sin_port         = htons(3000);

    if(bind(listenFd, (sockaddr *)&connFd, sizeof(connFd)) == -1)
    {
        std::cout << "bind error" << std::endl;
        return -1;
    }

    if(listen(listenFd, SOMAXCONN) == -1)
    {
        std::cout << "listen error;" << std::endl;
        //remember close opened fd
        close(listenFd);
        return -1;
    }

    std::vector<int> clientFds;
    int maxFd = listenFd;

    while (true)
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenFd,  &readSet);

        int cliNo = clientFds.size();
        for (int i = 0; i < cliNo; i++)
        {
            if (clientFds[i] == INVALID_FD)
            {
                FD_SET(clientFds[i], &readSet);
            }
        }

        timeval tm;
        tm.tv_sec  = 1;
        tm.tv_usec = 0;

        int ret = select(maxFd + 1, &readSet, NULL, NULL, &tm);
        if (ret == -1)
        {
            std::cout << "select error" << std::endl;
            if(errno != EINTR)
                break;
        } else if(ret == 0){
            //std::cout << "in time , no handle occour" << std::endl;
            continue;
        } else {
            if FD_ISSET(listenFd, &readSet)
            {
                sockaddr_in clientAddr;
                socklen_t   clientLen = sizeof(clientAddr);
              int clientFd = accept(listenFd, (sockaddr *)&clientAddr, &clientLen);
              if(clientFd == -1)
              {
                  break;
              } 

              std::cout << "accept a client socket: " << clientFd << std::endl;
              clientFds.push_back(clientFd);
              if(clientFd > maxFd)
              {
                  maxFd = clientFd;
              }

              else
              {
                  char recvbuf[64];
                  int clientFdsLen = clientFds.size();
                  for(int i = 0; i < clientFdsLen; i++)
                  {
                      if(clientFds[i] != INVALID_FD && FD_ISSET(clientFds[i], &readSet))
                      {
                          memset(recvbuf, 0, sizeof(recvbuf));
                          int recvLength = recv(clientFds[i], recvbuf, 64, 0);
                          if (recvLength <= 0 && errno != EINTR )
                          {
                              std::cout << "recv error" << clientFds[i] << std::endl;
                              close(clientFds[i]);
                              clientFds[i] = INVALID_FD;
                              continue;
                          }
                          std::cout << "clientfd is: " << clientFds[i] << "recvbuf is: " << recvbuf << std::endl;
                      }
                  }
              }
              
            } 
        }

        
    }

    int clientFdsLen = clientFds.size();
    for(int i = 0; i < clientFdsLen; i++)
    {
        clientFds[i] = INVALID_FD;
        close(clientFds[i]);
    }
    close(listenFd);
    
    return 0;
}