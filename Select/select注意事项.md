### select注意事项

#### select会改变原来绑定的监听标志位

- 调用select后原来FD_SET()所绑定的标志位可能已经不存在了；
- 调用select后，如果需要绑定原来的标志位，需要fd_set重新FD_ZERO后调用FD_SET重新绑定。
- 如果不重新绑定，再次调用select也不会检测到监听的描述符有变化；

#### linux中select会改变time_out的值

- linux中select原型 int select（int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout）;

  timeout 在linux中并不是const的， man中也可以查看相关说明；

- 所以如果需要复用timeout，需要重新初始化赋值；



#### time_out 不同赋值的含义

- NULL ，阻塞等待有一个描述符准备好；

- 在固定时间阻塞等待 描述符准备好；

- timeval两个成员变量都是0，轮询，直接返回；

  > 前两种 会被进程在等待期间捕获的信号中断， 代码判断  if(errno == EINTR)

