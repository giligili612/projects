# InetAddress
封装了`sockaddr_in`地址，并向外提供了获得其地址的接口

## 方法
### 公共方法
- `const char* ip()`：返回ip地址
- `uint16_t port()`：返回端口
- `const sockaddr* addr()`：返回`sockaddr*`指针，用于linux的socket编程
- `void setAddr(sockaddr_in addr)`：用`addr`改变其InetAddress地址
- 默认构造函数重载了许多版本，以适配传进来各种的参数都可以构造该类
### 私有方法
- `static uint16_t parse_port(const std::string& port_str)`：将传进来的port字符串转换为uint16_t
## 成员
- `sockaddr_in addr_`：底层IPv4的地址结构

# Socket
## 方法
### 公共方法
封装了Linux常用的Socket编程函数，包括listen、bind以及设置套接字属性的各函数
- `static const int CreateNonBlock()`：静态方法，创建一个非阻塞监听套接字，返回其fd
- 各种构造函数略
- 各种设置套接字属性的函数略
- `bool bind(InetAddress &addr)`：将Socket绑定到 ip:port，成功返回true
- `bool listen(int backlog)`：开始监听，成功返回true
- 返回套接字信息的方法（fd、ip、port）略
## 成员
- `int fd_`：套接字的fd
- `std::string ip_`：套接字绑定的ip
- `uint16_t port_`：套接字绑定的port

# Epoll
封装了底层epoll的操作
## 方法
### 公共方法
- 构造函数：epoll_create创建epoll并把fd赋值给`epollfd_`
- 析构函数：关闭epoll
- `int fd()`：返回底层epoll的fd
- `void updateChannel(Channel *ch)`：向Epoll里添加、更新监视的事件
- `void removeChannel(int fd)`：移除fd为传入参数的Channel
- `std::vector<Channel*> loop(int timeout = -1)`：返回在epoll树上的发生事件的所有channel

## 成员
- `int epollfd_`：管理epoll进程的FD
- `epoll_event events_[MaxEvents]`：事件数组

# Channel
负责IO的类
## 方法
- 构造函数：指向管理该Channel的epoll并指定Socket
- `uint32_t events()`：正在监视的事件
- `uint32_t rEvents()`：已发生的事件
- `bool isInEpoll()`：是否被加入了epoll
- `void setInEpoll(bool op)`：把Channel加入或离开epoll
- 各种设置`events_`、`revents_`属性或事件的函数
- `void handleEvent()`：处理读写事件，调用回调函数，使得上层不同的服务有不同的处理，接口设计模式
- 各种设置回调函数的函数
## 成员
- `int fd_`：标识一个socket或其他的IO资源
- `Epoll *epoll_`：这个Channel是挂在哪个Epoll上的
- `bool isInEpoll_`：标记自己是否在Epoll树上
- `uint32_t events_`：要监视的事件
- `uint32_t revents_`：已发生的事件
- 各种回调函数

# Connection
负责管理连接的类
## 方法
- 构造函数：指定管理Connection的EventLoop并移动给它一个指向Socket的unique_ptr，最后创建一个Channel来管理这个Socket之上的IO
- `void recvMessage()`：收到消息后的处理函数，会被设置为Channel的读事件的回调函数，处理完Buffer的数据后调用回调函数供上层的业务使用
- `void closeChannel()`：关闭Channel,然后关闭Connection
- `void errorChannel()`：同上
- `void write(const std::string &message)`：将消息写入Buffer,只允许IO线程写入
- `void send()`：把发送缓冲区的消息发给客户端
- `void sendInEventLoop(const std::string &message)`：如果当前线程是EventLoop的IO线程则写入Buffer，将处理完的消息写入输出Buffer，在此处加入消息头部（4字节的消息长度）。
## 成员
- `EventLoop *evLoop_`：指向Connection所属的EventLoop
- `std::unique_ptr<Socket> clientSocket_`：指向其管理的Socket
- `std::unique_ptr<Channel> clientChannel_`：指向其管理的Channel
- `Buffer recvBuf_`：接收缓冲区
- `Buffer sendBuf_`：发送缓冲区
- `std::atomic_bool disconnect_`：指定连接是否还在，线程安全
- 各种回调函数

# Acceptor
专门用于接收连接的类
## 方法
- `void acceptConnection()`：接收新连接，创建连接的fd,由TcpServer分配哪个事件循环保持这个连接
## 成员
- `EventLoop *evLoop_`：指向主事件循环
- `std::unique_ptr<Socket> socket_`：监听连接的Socket
- `std::function<void(std::unique_ptr<Socket>)> cnnCb`：分配Connection的回调函数
- `std::unique_ptr<Channel> listenChannel_`：接收连接的Channel

# Buffer
封装了底层的std::string
## 方法
- `const char *data()`：返回缓冲区的首地址
- `void clear()`：清空缓冲区
- `void append(const char *data, size_t size)`：向缓冲区的末尾里写数据
- `void erase(size_t pos , size_t num)`：清除缓冲区从pos开始指定num字节的数据
## 成员
- `std::string buffer_`：缓冲区本区

# ThreadPool
一般路过线程池
## 方法
- 构造函数：传入线程池的大小
- `void addTask(std::function<void()> task)`：把任务加入任务队列
- `void stop()`：发射停止信号，把所有还在线程池里的工作做完

## 成员
- `std::vector<std::thread> threads_`：线程池
- `std::queue<std::function<void()>> task_queue_`：任务队列
- `std::mutex mux_`：任务队列的同步锁，工作线程需获得锁才可以来任务队列拿任务
- `std::condition_variable cond_`：任务队列的信号量
- `bool stop_`：标志线程池是否还在运行

# EventLoop
事件循环
## 方法
- `void run()`：事件循环，不停的监听epoll上的事件
- `Epoll* epoll()`：底层的epoll
- `bool isEventLoop()`：检测线程是否在当前EventLoop里
- `void addIOTask(std::function<void()> fun)`：把IOtask加入到任务队列
- `void wakeup()`：唤醒当前线程来处理任务队列
- `void handleWakeup()`：处理任务队列里的任务
## 成员
- `Epoll *ep_`：底层epoll
- `pid_t thread_id_`：EventLoop的线程id
- `std::queue<std::function<void()>> queue_tasks_`：IO任务队列
- `std::mutex mutex_`：任务队列的锁
- `int wakeupfd_`：唤醒线程的fd
- `std::unique_ptr<Channel> wakeupChannel_`：监听wakeupfd_的Channel

# TcpServer
管理各个连接、事件循环的管家
## 方法
- 构造函数：指定服务器的ip、服务程序的端口、从线程数量
- 各种回调函数接口，供上层的业务类指定
- `void start()`：启动服务
- `void newConnection(std::unique_ptr<Socket> socket)`：创建连接并把连接存入哈希表
- `void closeConnection(int fd)`：清理连接表
- `void HandleData(shared_ptr_conn conn, std::string&)`：调用回调函数
## 成员
- `std::unique_ptr<EventLoop> mainLoop_`：主事件循环
- `std::vector<std::unique_ptr<EventLoop>> subLoops_`：从事件循环
- `ThreadPool thread_poll_`：线程池
- `size_t thread_num_`：线程池大小
- `std::unique_ptr<Acceptor> acceptor_`：接受连接的类
- `std::mutex connections_mutex_`：连接表的锁
- `std::map<int,shared_ptr_conn> connections_`：连接表
- 各种函数对象，供业务类回调