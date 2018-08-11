#include "TCPEventLoop.hpp"

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

TCPServer::TCPServer() : m_socklisten(0), m_run(true), m_maxconn(SOMAXCONN)
{}

TCPServer::TCPServer( const std::string& ip,
                      const unsigned int port) : TCPServer()
{
  bind(ip, port);
}

TCPServer::~TCPServer()
{}

void TCPServer::bind(const std::string &ip, const unsigned int port)
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

void TCPServer::stop()
{ m_run.store( false ); }

void TCPServer::serve()
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
        OnError(events[i].data.fd, "Polling error: Object pulled without event.");
        _tcp_close(events[i].data.fd);
        continue;
      }
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
          
          _tcp_create(infd);
          
          std::string ip; int port;
          _gethostinfo(in_addr, ip, port);
          
          OnConnect(infd, ip, port);
        }
        continue;
      }
      else
      {
        int fd = events[i].data.fd;
        
        while(true)
        {
          char buff[512]; ssize_t count;
          
          count = read(fd, buff, sizeof(buff));
          
          if( count == -1)
          {
            if( errno != EAGAIN )
            //{_tcp_close(fd); break;}
              throw std::runtime_error("TCP Event Loop: read() error");
            
            break;
          }
          else if( count == 0 )
          {
            {OnDisconnect(fd);}
            _tcp_close(fd);
            break;
          }
          
          {OnReceived(fd, buff, count);}
        }
      }
    }
    
    if( !m_closefds.empty() )
    {
      for( auto i : m_closefds )
        _tcp_close(i);
      
      m_closefds.clear();
    }
  }
  
  delete[] events;
  close(m_pollfd);
  close(m_socklisten);
}

void TCPServer::setMaxConn(int max) noexcept
{ m_maxconn = max; }

int TCPServer::getMaxConn() const noexcept
{ return m_maxconn; }

void TCPServer::_tcp_create(int fd)
{
  //Register in epoll watchlist
  struct epoll_event event;
  event.data.fd = fd;
  event.events = EPOLLIN | EPOLLET;
  if( epoll_ctl(m_pollfd, EPOLL_CTL_ADD, fd, &event) == -1 )
    throw std::runtime_error("epoll_ctl() error");
}

void TCPServer::_tcp_close(int fd)
{
  epoll_ctl(m_pollfd, EPOLL_CTL_DEL, fd, nullptr);
  close(fd);
}

void TCPServer::_gethostinfo(struct sockaddr &addr, std::string& ip, int& port)
{
  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
  getnameinfo(&addr, sizeof(addr),
              hbuf, sizeof(hbuf),
              sbuf, sizeof(sbuf),
              NI_NUMERICHOST | NI_NUMERICSERV);
  
  ip = hbuf;
  port = std::atoi(sbuf);
}
