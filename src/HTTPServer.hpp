#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include <iostream>
#include <string>
#include <map>

#include <sstream>

#include "TCPEventLoop.hpp"

namespace http
{
  void unescape(std::string&);
  
  enum class method
  {
    GET, POST, UNKNOWN
  };
  
  enum class status_code
  {
    OK=200,
    MOVED_PERMANENTLY=301,
    FORBIDDEN=403, NOT_FOUND=404,
    INTERNAL_SERVER_ERROR=500
  };
  
  class Request
  {
    friend class Parser;
  public:
    Request() : m_method(method::UNKNOWN), m_datalen(0), m_isHeaderComplete(false)
    {}
    
    const std::string   getPath() const noexcept
    {return m_uri;}
    
    const http::method  getMethod() const noexcept
    {return m_method;}
    
    const std::string   getVariable(const std::string &name) const noexcept
    {
      auto i = m_vars.find(name);
      return (i != m_vars.end() ? i->second : "" );
    }
    
    const std::string   getHeader(const std::string &name) const noexcept
    {
      auto i = m_headers.find(name);
      return (i != m_headers.end() ? i->second : "");
    }
    const std::string   getCookie(const std::string &name) const noexcept
    {
      auto i = m_cookies.find(name);
      return (i != m_cookies.end() ? i->second : "");
    }
    
    const std::string& getData() const noexcept
    {return m_data;}
    
    const ssize_t getDataLength() const noexcept
    {return m_datalen;}
    
  private:
    std::map<std::string, std::string> m_headers, m_cookies, m_vars;
    std::string m_uri, m_data;
    http::method m_method;
    
    size_t m_datalen; bool m_isHeaderComplete;
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
    friend class Server;
  public:
    Client(int);
    
    Client& operator <<(const Response&);
    Client& send(const Response&);
    
  private:
    int m_fd;
    Request* m_incomming;
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
      
      void http_close_request(http::Client& cli)
      { _http_close(cli); }
      
      //HTTP Callbacks
      virtual void OnPost(http::Client&, const http::Request&);
      virtual void OnGet(http::Client&, const http::Request&);
    
    private:
    //void _parse(Client&, const char*, const ssize_t);
    void _http_close(http::Client&);
    std::map<int, Client*> m_clients;
  };
  
  class Parser
  {
  public:
    Parser();
    Parser(Request*);
    
    ~Parser();
    
    void setRequest(Request*) noexcept;
    void write(const char*, const ssize_t);
    
    bool isComplete() const noexcept;
  private:
    void _decodeVariable();
    Request* m_target;
  };
}
#endif //HTTPSERVER_HPP
