#include "Audio-Bridge-Server.hpp"

AudioServer::AudioServer() : m_db(nullptr)
{}

AudioServer::AudioServer( const std::string &ip, const unsigned int port ) : http::Server(ip,port)
{m_db=nullptr;}

AudioServer::~AudioServer()
{}

void AudioServer::setDBController( AudioDatabase *db )
{m_db = db;}

AudioDatabase* AudioServer::getDBController()
{return m_db;}

void AudioServer::OnPost( http::Client &client, const http::Request &req)
{
  auto request = req.getPath();
  
  if( request == "/Radio2/FilterBridge.php" )
    _audiobridge_process(client, req);
  else if( request == "/api/filter/" )
    _api_filter( client, req );
  else if( request == "/api/filter/composer" )
    ;//_api_filter_composer( client, req );
  else
  {
    http::Response resp;
    resp.setStatusCode(http::status_code::FORBIDDEN);
    resp.appendData("Wrong Post request");
    client << resp;
  }
}

void AudioServer::OnGet( http::Client &client, const http::Request &req)
{
  if( req.getPath() == "/crossdomain.xml" )
    _audiobridge_sendCD(client);
  else if( req.getPath().rfind(".mp3") != std::string::npos )
    _audiobridge_getmp3(client, req);
  else
  {
    http::Response resp;
    resp.setStatusCode( http::status_code::NOT_FOUND );
    client << resp;
  }
}

void AudioServer::_audiobridge_process(http::Client &client, const http::Request &req)
{
  http::Response resp; AudioBridgeFilter filter;
  filter.set(req);
  
  if( filter.validate() )
  {
    AudioQueryResult rslt = m_db->getViaFilter(filter);
    
    resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Content-Type", "application/json");
    
    resp.appendData("{\"ResultSet\":");
    resp.appendData(rslt.toJson());
    resp.appendData("}");
  }
  else
  {
    resp.setStatusCode(http::status_code::INTERNAL_SERVER_ERROR);
    resp.addHeader("Content-Type", "text/plain");
    resp.appendData("Process Error");
  }
  
  client << resp;
}

void AudioServer::_audiobridge_getmp3(http::Client &client, const http::Request &req)
{
  std::string num = req.getPath(); http::Response resp;
  if( m_db )
  {
    std::string songid; std::string::size_type pos1, pos2;
      pos1 = num.rfind('/') + 1;
      pos2 = num.length() - 4;
    songid = num.substr( pos1, pos2 - pos1);
    
    std::cout << "Looking for sond ID: " << songid << std::endl;
    
    AudioQueryResult rslt = m_db->getSongByID( std::atoi(songid.c_str()) );
    
    if( rslt.isEmpty() )
    {
      resp.setStatusCode(http::status_code::NOT_FOUND);
      resp.addHeader("Content-Type", "text/html");
      resp.appendData("Song not found in Index DB");
    }
    else
    {
      resp.setStatusCode(http::status_code::MOVED_PERMANENTLY);
      resp.addHeader( "Location", rslt[0].getURL() );
      client << resp;
      return;
    }
  }
  else
  {
    resp.setStatusCode( http::status_code::INTERNAL_SERVER_ERROR );
    resp.addHeader("Content-Type", "text/html");
    resp.appendData("Song Index unavailable.");
  }
}

void AudioServer::_audiobridge_sendCD(http::Client &client)
{
  http::Response resp;
  resp.setStatusCode(http::status_code::OK);
  resp.addHeader("Content-Type", "text/xml");
  
  resp.appendData("<?xml version=\"1.0\"?>");
  resp.appendData("<!DOCTYPE cross-domain-policy SYSTEM \"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">");
  resp.appendData("<cross-domain-policy>");
  resp.appendData("    <site-control permitted-cross-domain-policies=\"master-only\"/>");
  resp.appendData("    <allow-access-from domain=\"*\" secure=\"false\"/>");
  resp.appendData("    <allow-http-request-headers-from domain=\"*\" headers=\"*\"/>");
  resp.appendData("</cross-domain-policy>");
  
  client << resp;
}

bool AudioServer::_audiobridge_JSONprocess(const http::Request &req, std::string &result)
{
  AudioBridgeFilter filter; filter.set(req);
  
  if( !filter.validate() ) return false;
  
  AudioQueryResult rslt = m_db->getViaFilter(filter);
  result = rslt.toJson();
  
  return true;
}

void AudioServer::_api_filter(http::Client &client, const http::Request &req)
{
  
}
