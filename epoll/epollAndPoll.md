poll 对加入的描述符进行遍历， 选择出满足条件的描述符；
epoll返回满足条件的描述符列表；
epoll适合链接数目较多但是活跃数目少；