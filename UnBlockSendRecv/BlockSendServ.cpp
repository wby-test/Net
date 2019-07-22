#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT  3000

int main(int argc, char **argv)
{
    int listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenFd == -1)
    {
        std::cout << "create socket error " << strerror(errno) << std::endl;
        return -1;
    }

    sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(listenFd, (sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cout << "bind error  " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

    if(listen(listenFd, SOMAXCONN) == -1)
    {
        std::cout << "listen error: " << strerror(errno) << std::endl;
        close(listenFd);
        return -1;
    }

	sockaddr_in clientAddr;
	socklen_t clientLen = sizeof(clientAddr);
	int acceptFd = accept(listenFd, (sockaddr *)&clientAddr, &clientLen);

	while(true)
	{
		if(acceptFd != -1)
		{
			std::cout << "accept a client connection: " << acceptFd << std::endl;
		}
	}
			             

#if 0
    while(true)
    {
        sockaddr_in clientAddr;
        socklen_t  clientLen = sizeof(clientAddr);
        int acceptFd = accept(listenFd, (sockaddr *)&clientAddr, &clientLen);
        if(acceptFd != -1)
        {
			std::cout << "accept a client connection: " << acceptFd << std::endl;
        }
    }
#endif
    close(listenFd);
    return 0;
}
