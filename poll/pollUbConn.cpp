#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#define POLL


/*
*不做任何数据收发处理，只验证非阻塞connect是否成功
*/
int main()
{
	int clientFd = socket(AF_INET, SOCK_STREAM, 0);
	if(clientFd == -1)
	{
		std::cout << "create socket err: " << strerror(errno) << std::endl;
		return -1;
	}
	
	int oldSockFlags = fcntl(clientFd, F_GETFL, 0);
	int newSockFlags = oldSockFlags | O_NONBLOCK;
	if(fcntl(clientFd, F_SETFL, newSockFlags) == -1)
	{
		std::cout << "set nonblock error: " << strerror(errno) << std::endl;
		close(clientFd);
		return -1;
	}
	
	struct sockaddr_in serverAddr;
	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(3000);
	
	for(; ;)
	{
		int iRet = connect(clientFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
		
		if(iRet == 0)
		{
			std::cout << "connect success: " << std::endl;
			return 0;
		}
		else if(iRet == -1)
		{
			if(errno == EINTR)
			{
				std::cout << " connecting  is interupted by signal" << std::endl;
				continue;
			}
			else if(errno == EINPROGRESS)
			{
				//非阻塞connect，尝试重新连接
				break;
			}
			else
			{
				close(clientFd);
				std::cout << "connect error, not in poll" << strerror(errno) <<  std::endl;
				return -1;
			}
		}	
	}

#ifdef POLL
	pollfd event;
	event.fd = clientFd;
	event.events = POLLOUT;
	int timeout = 3000;
	
	if(poll(&event, 1, timeout != 1))
	{
		close(clientFd);
		std::cout << "[poll] connect server error" << strerror(errno) << std::endl;
		return -1;
	}
	
	if(!(event.revents & POLLOUT))
	{
		close(clientFd);
		std::cout << "[poll return not pollout] connect server error" << strerror(errno) << std::endl;
		return -1;
	}
	
	
	int err;
    socklen_t len = static_cast<socklen_t>(sizeof err);
    if (::getsockopt(clientFd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
        return -1;
        
    if (err == 0)
        std::cout << "connect to server successfully." << std::endl;
    else
    	std::cout << "connect to server error." << strerror(errno) << std::endl;
#endif

#ifdef SELECT
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(clientFd, &writeset);
	//可以利用tv_sec和tv_usec做更小精度的超时控制
    struct timeval tv;
    tv.tv_sec = 3;  
    tv.tv_usec = 0;
    if (select(clientFd + 1, NULL, &writeset, NULL, &tv) == 1)
    {
		std::cout << "[select] connect to server successfully." << std::endl;
	} else {
		std::cout << "[select] connect to server error." << std::endl;
	}
#endif


	//5. 关闭socket
	close(clientFd);	
	return 0;
}