#include "HTTPServer.hpp"

static std::map<http::status_code, const std::string> _stat_code_to_str = 
{{
  {http::status_code::OK, "HTTP/1.1 200 OK"},
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
  http::Request request(buff, size);
  
  switch( request.getMethod() )
  {
    case http::method::GET:
      this->OnGet(*m_clients[fd], request);
      break;
    
    case http::method::POST:
      this->OnPost(*m_clients[fd], request);
      break;
    
    case http::method::UNKNOWN:
    default:
    std::cout << "Paquet dropped" << std::endl;
  }
}

void http::Server::OnError(int, const std::string &err)
{
  std::cerr << "Something went wrong: " << err << std::endl;
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
  
  resp.appendData("Working.");
  
  client << resp;
}

void http::Server::OnGet(http::Client &client, const http::Request &req)
{
  http::Response resp;
  resp.setStatusCode(http::status_code::OK);
  
  resp.appendData("Working.");
  
  client << resp;
}

void http::Request::_parse(const char *buff, const ssize_t len)
{
  std::string data(buff, len);
  std::istringstream stream(data);
  
  while(!stream.eof())
  {
    std::string line; std::getline(stream, line);
    if( line == "\r" ) //End of header or end packet
      break;
    
    if( line.find("GET") != std::string::npos || line.find("POST") != std::string::npos )
    {
      std::istringstream i(line);
      std::string method;
      
      i >> method >> m_uri;
      if(method == "GET")
        m_method = http::method::GET;
      else if( method == "POST" )
        m_method = http::method::POST;
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
          
          m_cookies[name] = value;
        }
      }
      else
        m_headers[key] = data;
    }
  }
}

http::Response::Response(){}

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

http::Client::Client(int fd) : m_fd(fd)
{}

http::Client& http::Client::operator <<( const http::Response &resp)
{
  return send(resp);
}

http::Client& http::Client::send( const http::Response &resp)
{
  std::ostringstream raw_data;
  
  raw_data << _stat_code_to_str[ resp.m_code ] << "\r\n";
  
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

/*
std::ostream& operator<<(std::ostream &out, const http::Request &req)
{
  out << "Method: ";
  switch(req.m_method)
  {
    case http::method::GET:
      out << "GET";
      break;
    case http::method::POST:
      out << "POST";
      break;
    
    case http::method::UNKNOWN:
    default:
      out << "UNKNOWN";
      break;
  }
  
  out << std::endl << "Rquested URI: " << req.m_uri << std::endl
      << "-- HEADERS -- " << std::endl;
  for( auto i : req.m_headers )
    out << std::get<0>(i) << ": " << std::get<1>(i) << std::endl;
  
  if( !req.m_cookies.empty() )
  {
    out << std::endl << "-- Cookies --" << std::endl;
    for(auto i : req.m_cookies )
      out << std::get<0>(i) << ": " << std::get<1>(i) << std::endl;
  }
  
  out << std::endl;
  
  return out;
}
*/
