#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <stdexcept>

#include <string>
#include <map>

#include <atomic>

extern "C"
{
  #include <sys/epoll.h>
  
  #include <arpa/inet.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  
  #include <fcntl.h>
  
  #include <unistd.h>
}

bool sock_non_block(int fd) noexcept
{
  int flags = fcntl( fd, F_GETFL, 0 );
  if(flags == -1)
    return false;
  
  flags |= O_NONBLOCK;
  if( fcntl( fd, F_SETFL, flags ) == -1 )
    return false;
  
  return true;
}


template<class T>
class TCPServer
{
public:
  TCPServer();
  TCPServer(const std::string&, const unsigned int);
  ~TCPServer();
  
  void serve();
  void stop();
  
  void bind( const std::string&, const unsigned int );
  
  void  setMaxConn(int) noexcept;
  int   getMaxConn() const noexcept;
  
  const bool isRunning() const
  {return m_run;}
  
  void OnConnect(T&, std::string& ip, int& port){}
  
  void OnReceived(T&, char* buff, const ssize_t size){}
  /*void OnSend(T&){}*/
  
  void OnError(T&, const std::string&){}
  void OnDisconnect(T&){}
  
private:
  int m_socklisten, m_pollfd;
  std::atomic<bool> m_run;
  std::map<T*, int> m_data;
  
  T* _create(int);
  void _close(T*);
  void _gethostinfo(struct sockaddr&, std::string&, int&);
  
  int m_maxconn;
};

template<class T>
TCPServer<T>::TCPServer() : m_socklisten(0), m_run(true), m_maxconn(SOMAXCONN)
{}

template<class T>
TCPServer<T>::TCPServer(  const std::string& ip,
                          const unsigned int port) : TCPServer()
{
  bind(ip, port);
}

template<class T>
TCPServer<T>::~TCPServer()
{}

template<class T>
void TCPServer<T>::stop()
{ m_run.store( false ); }

template<class T>
void TCPServer<T>::serve()
{
  struct epoll_event event;
  struct epoll_event *events;
  
  events = new struct epoll_event[64];
  
  m_pollfd = epoll_create1(0);
  if( m_pollfd == -1 )
    throw std::runtime_error("epoll_create1(): Error");
  
  event.data.fd = m_socklisten;
  event.events = EPOLLIN | EPOLLET;
  
  if( epoll_ctl(m_pollfd, EPOLL_CTL_ADD, m_socklisten, &event) == -1)
    throw std::runtime_error("epoll_ctl(): error");
  
  while(m_run)
  {
    int n;
    n = epoll_wait( m_pollfd, events, 64, 500 );
    for( int i = 0; i < n; i++ )
    {
      if( (events[i].events & EPOLLERR) ||
          (events[i].events & EPOLLHUP) ||
          !(events[i].events & EPOLLIN) )
      {
        T *ptr = static_cast<T*>(events[i].data.ptr);
        OnError(*ptr, "Polling error: Object pulled without event.");
        _close(ptr);
        continue;
      }
      //TODO: Fix this ... Mixing FD / PTR is risky. (data is an union)
      else if( events[i].data.fd == m_socklisten )
      {
        while(true)
        {
          struct sockaddr in_addr; socklen_t in_len = sizeof(struct sockaddr);
          int infd;
          
          infd = accept( m_socklisten, &in_addr, &in_len );
          if( infd == -1 )
          {
            if( (errno == EAGAIN) ||
                (errno == EWOULDBLOCK) )
              break;
            else
              throw std::runtime_error("accept() error in event loop.");
          }
          
          if( !sock_non_block(infd) )
            throw std::runtime_error("sock_non_block(): error");
          
          T *new_object = _create(infd);
          
          std::string ip; int port;
          _gethostinfo(in_addr, ip, port);
          
          OnConnect(*new_object, ip, port);
        }
        continue;
      }
      else
      {
        T *ptr = static_cast<T*>(events[i].data.ptr);
        bool done = false;
        
        while(true)
        {
          char buff[512]; ssize_t count;
          
          count = read(m_data[ptr], buff, sizeof(buff));
          
          if( count == -1)
          {
            if( errno != EAGAIN )
              throw std::runtime_error("TCPServer Loop: read() error");
            
            break;
          }
          else if( count == 0 )
          {
            done = true;
            break;
          }
          
          {OnReceived(*ptr, buff, count);}
        }
        
        if(done)
        {
          {OnDisconnect(*ptr);}
          _close(ptr);
        }
      }
    }
  }
  
  delete[] events;
  close(m_pollfd);
  close(m_socklisten);
}

template<class T>
void TCPServer<T>::bind(const std::string &ip, const unsigned int port)
{
  struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
  
  m_socklisten = socket(AF_INET, SOCK_STREAM, 0);
  if( m_socklisten < 0 )
    throw std::runtime_error("socket(): error");
  
  int turnon = 1;
  setsockopt(m_socklisten, SOL_SOCKET, SO_REUSEADDR, &turnon, sizeof(turnon));
  
  if( ::bind(m_socklisten, (struct sockaddr*)&addr, sizeof(addr)) < 0 )
    throw std::runtime_error("bind(): error");
  
  if( listen(m_socklisten, m_maxconn) < 0 )
    throw std::runtime_error("listen(): error");
  
  if( !sock_non_block(m_socklisten) )
    throw std::runtime_error("sock_non_block(): error");
}

template<class T>
void TCPServer<T>::setMaxConn(int max) noexcept
{ m_maxconn = max; }

template<class T>
int TCPServer<T>::getMaxConn() const noexcept
{ return m_maxconn; }

template<class T>
T* TCPServer<T>::_create(int fd)
{
  //Create object & Register in internal vector
  T* ptr = new T(fd);
  m_data[ptr] = fd;
  
  //Register in epoll watchlist
  struct epoll_event event;
  event.data.ptr = ptr;
  event.events = EPOLLIN | EPOLLET;
  if( epoll_ctl(m_pollfd, EPOLL_CTL_ADD, fd, &event) == -1 )
    throw std::runtime_error("epoll_ctl() error");
  
  return ptr;
}

template<class T>
void TCPServer<T>::_close(T* ptr)
{
  epoll_ctl(m_pollfd, EPOLL_CTL_DEL, m_data[ptr], nullptr);
  m_data.erase(ptr);
  
  delete ptr;
}

template<class T>
void TCPServer<T>::_gethostinfo(struct sockaddr &addr, std::string& ip, int& port)
{
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  getnameinfo(&addr, sizeof(addr),
              hbuf, sizeof(hbuf),
              sbuf, sizeof(sbuf),
              NI_NUMERICHOST | NI_NUMERICSERV);
  
  ip = hbuf;
  port = std::atoi(sbuf);
}

#endif //TCPSERVER_HPP
