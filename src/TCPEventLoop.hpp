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

bool sock_non_block(int fd) noexcept;

class TCPServer
{
public:
  TCPServer();
  TCPServer(const std::string&, const unsigned int);
  virtual ~TCPServer();
  
  void serve();
  void stop();
  
  void bind( const std::string&, const unsigned int );
  
  void  setMaxConn(int) noexcept;
  int   getMaxConn() const noexcept;
  
  const bool isRunning() const
  {return m_run;}
  
  virtual void OnConnect(int, const std::string&, const int){}
  
  virtual void OnReceived(int, char* buff, const ssize_t size){}
  
  virtual void OnError(int, const std::string&){}
  virtual void OnDisconnect(int){}
  
private:
  int m_socklisten, m_pollfd;
  std::atomic<bool> m_run;
  
  void _create(int);
  void _close(int);
  void _gethostinfo(struct sockaddr&, std::string&, int&);
  
  int m_maxconn;
};

#endif //TCPSERVER_HPP
