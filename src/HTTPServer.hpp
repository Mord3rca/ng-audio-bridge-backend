#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <iostream>
#include <string>
#include <map>

#include <sstream>

#include "TCPEventLoop.hpp"

namespace http
{
  enum class method
  {
    GET, POST, UNKNOWN
  };
  
  enum class status_code
  {
    OK=200,
    FORBIDDEN=403, NOT_FOUND=404,
    INTERNAL_SERVER_ERROR=500
  };
  
  class Request
  {
  public:
    Request() : m_method(method::UNKNOWN) {}
    Request(const char *buff, const ssize_t len)
    {_parse(buff, len);}
    
    const std::string   getUri() const noexcept
    {return m_uri;}
    
    const http::method  getMethod() const noexcept
    {return m_method;}
    
    const std::string   getHeader(const std::string &name) const noexcept
    {
      for(auto i : m_headers)
        if( std::get<0>(i) == name )
          return std::get<1>(i);
      
      return "";
    }
    const std::string   getCookie(const std::string &name) const noexcept
    {
      for(auto i : m_cookies)
        if( std::get<0>(i) == name )
          return std::get<1>(i);
      
      return "";
    }
    
  private:
    //friend std::ostream& operator<<(std::ostream&, const http::Request&);
    void _parse(const char*, const ssize_t);
    
    std::map<std::string, std::string> m_headers, m_cookies;
    std::string m_uri, m_data;
    http::method m_method;
  };
  
  class Response
  {
    friend class Client;
  public:
    Response();
    
    void setStatusCode(enum http::status_code );
    
    void addHeader(const std::string&, const std::string&);
    void removeHeader(const std::string&);
    
    void appendData(const std::string&);
    void clearData();
    
  private:
    std::map<std::string, std::string> m_headers;
    std::string m_data; http::status_code m_code;
  };
  
  //Will be used for more advanced shit. Like Auth etc
  class Client
  {
  public:
    Client(int);
    
    Client& operator <<(const Response&);
    Client& send(const Response&);
    
  private:
    int m_fd;
  };

  class Server : public TCPServer
  {
    public:
      Server();
      Server(const std::string&, const unsigned int);
      ~Server();
      
      //TCP callbacks
      void OnConnect(int, const std::string &ip, const int) override;
      void OnReceived(int, char*, const ssize_t) override;
      void OnError(int, const std::string&) override;
      void OnDisconnect(int) override;
      
      //HTTP Callbacks
      void OnPost(http::Client&, const Request&);
      void OnGet(http::Client&, const Request&);
    
  private:
    std::map<int, Client*> m_clients;
  };
}
#endif //HTTPSERVER_HPP
