#ifndef ARGS_HPP
#define ARGS_HPP

#include <iostream>
#include <string>

extern "C"
{
  #include <unistd.h>
  #include <getopt.h>
}

void printHelp( std::ostream&, const char* = nullptr ) noexcept;

class cArg
{
public:
  cArg(int, char**);
  ~cArg();

  const bool daemonize() const noexcept;
  
  const std::string& getUserName() const noexcept;
  const std::string& getGroupName() const noexcept;
  
  const std::string& getDBPath() const noexcept;
  const bool         isLive() const noexcept;
  
  const int getListeningPort() const noexcept;
  const std::string& getListenIP() const noexcept;
  
private:
  void _parse(int, char**);
  
  bool m_isDaemon;
  std::string m_db_path; bool m_live;
  std::string m_user, m_group;
  std::string m_ip; int m_port;
};

#endif //ARGS_HPP
