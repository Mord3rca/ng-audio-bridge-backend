#include "HTTPServer.hpp"

static const std::map<const http::status_code, const std::string> _stat_code_to_str = 
{{
  {http::status_code::OK, "HTTP/1.1 200 OK"},
  {http::status_code::MOVED_PERMANENTLY, "HTTP/1.1 301 Moved Permanently"},
  {http::status_code::FORBIDDEN, "HTTP/1.1 403 Forbidden"},
  {http::status_code::NOT_FOUND, "HTTP/1.1 404 Not Found"},
  {http::status_code::INTERNAL_SERVER_ERROR, "HTTP/1.1 500 Internal Server Error"}
}};

http::Server::Server()
{}

http::Server::Server(const std::string &ip, const unsigned port) : TCPServer(ip, port)
{}

http::Server::~Server()
{
  for(auto i : m_clients)
    delete std::get<1>(i);
  
  this->stop();
}

//TCP Callbacks
void http::Server::OnConnect(int fd, const std::string &ip, const int port)
{
  std::cout << "Client connected: " << ip << ":" << port << std::endl;
  m_clients[fd] = new http::Client(fd);
}

void http::Server::OnReceived(int fd, char *buff, const ssize_t size)
{
  http::Client* cli = m_clients[fd]; http::Request* req;
  
  if( !cli->m_incomming ) cli->m_incomming = new http::Request();
  req = cli->m_incomming;
  
  http::Parser writer(req);
  writer.write( buff, size );
  
  if( writer.isComplete() )
  {
    switch( req->getMethod() )
    {
      case http::method::GET:
        this->OnGet(*cli, *req);
        break;
      
      case http::method::POST:
        this->OnPost(*cli, *req);
        break;
      
      case http::method::UNKNOWN:
      default:
      std::cout << "Paquet dropped" << std::endl;
    }
    std::string connection_policy = req->getHeader("Connection");
    if( connection_policy.find("Keep-Alive") == std::string::npos ||
        connection_policy.find("keep-alive") == std::string::npos )
      http_close_request(*cli);
    
    delete req; cli->m_incomming = nullptr;
  }
}

void http::Server::OnError(int fd, const std::string &err)
{
  std::cerr << "Something went wrong: " << err << std::endl;
  
  auto i = m_clients.find(fd);
  m_clients.erase(fd);
  delete i->second;
}

void http::Server::OnDisconnect(int fd)
{
  std::cout << "Connection terminated.." << std::endl;
  auto i = m_clients.find(fd);
  m_clients.erase(i);
  delete i->second;
}

//Default HTTP CallBacks:
void http::Server::OnPost(http::Client &client, const http::Request &req)
{
  http::Response resp;
  resp.setStatusCode(http::status_code::OK);
  resp.addHeader("Content-Type", "text/plain");
  
  resp.appendData("Working.");
  
  client << resp;
}

void http::Server::OnGet(http::Client &client, const http::Request &req)
{
  http::Response resp;
  resp.setStatusCode(http::status_code::OK);
  resp.addHeader("Content-Type", "text/plain");
  
  resp.appendData("Working.");
  
  client << resp;
}

http::Response::Response(){}

http::Response::Response(const http::status_code code, const std::map<std::string, std::string> headers, const std::string data) :
  m_headers(headers), m_data(data), m_code(code)
{}

void http::Response::setStatusCode( http::status_code code)
{
  m_code = code;
}

void http::Response::addHeader(const std::string &name, const std::string &value)
{
  m_headers[name] = value;
}

void http::Response::removeHeader(const std::string &name)
{
  auto i = m_headers.find(name);
  if( i != m_headers.end() )
    m_headers.erase(i);
}

void http::Response::appendData(const std::string &data)
{
  m_data += data;
}

void http::Response::clearData()
{ m_data.clear(); }

http::Client::Client(int fd) : m_fd(fd), m_incomming(nullptr)
{}

http::Client& http::Client::operator <<( const http::Response &resp)
{
  return send(resp);
}

http::Client& http::Client::send( const http::Response &resp)
{
  std::ostringstream raw_data;
  
  raw_data << _stat_code_to_str.find( resp.m_code )->second << "\r\n";
  
  for(auto i : resp.m_headers)
    raw_data << std::get<0>(i) << ": " << std::get<1>(i) << "\r\n";
  
  if( !resp.m_data.empty() )
  {
    raw_data << "Content-Length: " << resp.m_data.length() << "\r\n";
  }
  
  raw_data << "\r\n" << resp.m_data;
  
  write(m_fd, raw_data.str().c_str(), raw_data.str().length());
  
  return *this;
}

void http::Server::_http_close( http::Client& cli )
{
  for( auto i : m_clients )
    if( std::get<1>(i) == &cli )
      tcp_close_request( std::get<0>(i) );
}

http::Parser::Parser() : m_target(nullptr) {}
http::Parser::Parser(http::Request *req) : m_target(req) {}

http::Parser::~Parser(){}

void http::Parser::setRequest( http::Request *req ) noexcept
{ m_target = req; }

bool http::Parser::isComplete() const noexcept
{
  return m_target->m_isHeaderComplete &&
  ( (m_target->m_datalen == 0) ? (true) : ( m_target->m_data.length() == m_target->m_datalen ) );
}

void http::Parser::write( const char *buff, const ssize_t len )
{
  if( !m_target ) return;
  
  std::string strbuff(buff, len);
  std::istringstream stream(strbuff);
  
  while( stream.good() && (!stream.eof() || !this->isComplete()) )
  {
    if( !m_target->m_isHeaderComplete )
    {
      std::string line;
      std::getline(stream, line);
      if( line == "\r" )
      {
        m_target->m_isHeaderComplete = true;
        if( !m_target->getHeader("Content-Length").empty() )
          m_target->m_datalen = std::strtoul(m_target->getHeader("Content-Length").c_str(), nullptr, 0);
        
        continue;
      }
      
      if( line.find("GET") != std::string::npos || line.find("POST") != std::string::npos )
      {
        std::istringstream i(line);
        std::string method;
        
        i >> method >> m_target->m_uri;
        if(method == "GET")
          m_target->m_method = http::method::GET;
        else if( method == "POST" )
          m_target->m_method = http::method::POST;
      }
      else
      {
        std::istringstream i(line);
        std::string key, data;
        
        std::getline(i, key, ':');
        std::getline(i, data, '\r');
        
        if(key == "Cookie")
        {
          std::istringstream k(data);
          while( !k.eof() )
          {
            std::string entity, name, value;
            std::getline(k, entity, ';');
            
            std::string::size_type delim = entity.find('=');
            name  = entity.substr(0, delim);
            value = entity.substr(delim+1);
            
            //TODO: Better function PLZ
            if( name[0] == ' ' )
              name = name.substr(1);
            
            m_target->m_cookies[name] = value;
          }
        }
        else
          m_target->m_headers[key] = data;
      }
    }
    else
    {
      if( this->isComplete() ) break;
      
      m_target->m_data += stream.str().substr( stream.tellg() );
      break;
    }
  }
  
  if( this->isComplete() ) _decodeVariable();
}

void http::Parser::_decodeVariable()
{
  std::string data;
  switch( m_target->m_method )
  {
    case http::method::GET:
      {
        auto t = m_target->m_uri.find('?');
        if( t != std::string::npos )
        {
          data = m_target->m_uri.substr(t+1);
          m_target->m_uri = m_target->m_uri.substr(0, t);
        }
        else
          return;
      }
      break;
    case http::method::POST:
      data = m_target->m_data;
      break;
    default: return;
  }
  
  std::istringstream stream(data);
  do
  {
    std::string line, key, value; std::string::size_type p;
    std::getline( stream, line, '&' );
    
    p = line.find("=");
    if( p != std::string::npos )
    {
      key = line.substr(0, p);  http::unescape(key);
      value = line.substr(p+1); http::unescape(value);
      
      m_target->m_vars[key] = value;
    }
    else
      m_target->m_vars[line] = "";
    
  } while( stream.good() && !stream.eof() );
}

void http::unescape( std::string &str)
{
  std::string result;
  //TODO: Error handling, just in case.
  for( size_t i = 0; i < str.length(); i++ )
  {
    if( str[i] == '%' )
    {
      char u = static_cast<char>(std::strtol( str.substr(i+1, 2).c_str(), nullptr, 16 ));
      result += u;
      i+=2;
    }
    else
      result += str[i];
  }
  str = result;
}
