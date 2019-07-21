#include <sys/types.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <string.h>

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     3000

int main(int argc, char* argv[])
{
    //创建一个socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1)
    {
        std::cout << "create client socket error." << std::endl;
        return -1;
    }

    //连接服务器
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    serveraddr.sin_port = htons(SERVER_PORT);
    if (connect(clientfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        std::cout << "connect socket error." << std::endl;
		close(clientfd);
        return -1;
    }
    int count = 0;
	while (true)
	{
        timeval tm;
	    tm.tv_sec = 5;
	    tm.tv_usec = 0;	
        
        fd_set readset;
	    FD_ZERO(&readset);
	
	//将侦听socket加入到待检测的可读事件中去
	    FD_SET(clientfd, &readset);	

        int ret;
     
        fd_set backup_readset;
        memcpy(&backup_readset, &readset, sizeof(fd_set));
            
        //暂且只检测可读事件，不检测可写和异常事件
        ret = select(clientfd + 1, &readset, NULL, NULL, NULL);
        std::cout << "tm.tv_sec: " << tm.tv_sec << ", tm.tv_usec: " << tm.tv_usec << std::endl;
        std::cout << "begin time: " <<  "tm.tv_sec: " << tm.tv_sec << ", tm.tv_usec: " << tm.tv_usec << std::endl;
		if (memcmp(&readset, &backup_readset, sizeof(fd_set)) == 0)
		{
			std::cout << "equal" << std::endl;
		}
		else
		{
			std::cout << "not equal" << std::endl;
		}
        if (ret == -1)
        {
            //除了被信号中断的情形，其他情况都是出错
            if (errno != EINTR)
                break;
        } else if (ret == 0){
            //select函数超时
            std::cout << "no event in specific time interval, count：" << count << std::endl;
            ++count;
            std::cout << "\n\n" << std::endl;
            continue;
        } else {
            if (FD_ISSET(clientfd, &readset))
            {
                //检测到可读事件
                char recvbuf[32];
                memset(recvbuf, 0, sizeof(recvbuf));
                //假设对端发数据的时候不超过31个字符。
                int n = recv(clientfd, recvbuf, 32, 0);
                if (n < 0)
                {
                    //除了被信号中断的情形，其他情况都是出错
                    if (errno != EINTR)
                        break;
                } else if (n == 0) {
                    //对端关闭了连接
                    std::cout << "peer close connection " << std::endl;
                    break;
                } else {

                    std::cout << "recv data: " << recvbuf << std::endl;
                }
            }
            else 
            {
                std::cout << "other socket event." << std::endl;
            }
        }
        
	}		
	
	//关闭socket
	close(clientfd);

    return 0;
}