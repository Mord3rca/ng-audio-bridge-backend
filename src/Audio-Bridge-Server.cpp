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
  if( req.getUri() == "/Radio2/FilterBridge.php" )
    _audiobridge_process(client, req);
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
  if( req.getUri() == "/crossdomain.xml" )
    _audiobridge_sendCD(client);
  else if( req.getUri().rfind(".mp3") != std::string::npos )
    _audiobridge_getmp3(client, req);
  else
  {
    http::Response resp;
    resp.setStatusCode( http::status_code::NOT_FOUND );
    client << resp;
    http_close_request(client);
  }
}

void AudioServer::_audiobridge_process(http::Client &client, const http::Request &req)
{
  std::string json_rslt; http::Response resp;
  if( _audiobridge_JSONprocess(req, json_rslt) )
  {
    resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Content-Type", "application/json");
    
    resp.appendData("{\"ResultSet\":");
    resp.appendData(json_rslt);
    resp.appendData("}");
    
    std::cout << "Query Result: " << json_rslt << std::endl;
  }
  else
  {
    resp.setStatusCode(http::status_code::INTERNAL_SERVER_ERROR);
    resp.appendData("Process Error");
  }
  
  client << resp;
}

void AudioServer::_audiobridge_getmp3(http::Client &client, const http::Request &req)
{
  std::string num = req.getUri(); http::Response resp;
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
      http_close_request(client);
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
  Json::CharReaderBuilder builder; Json::Value root;
  std::string jsonstr = req.getData().substr(11); //Removing filterJSON=
  http::unescape(jsonstr);

  Json::CharReader *json_read = builder.newCharReader();
  std::string err;

  if( !json_read->parse( jsonstr.c_str(), jsonstr.c_str() + jsonstr.length() , &root, &err) )
  {
    delete json_read;
    return false;
  }
  
  delete json_read;
  
  filter filt(root);
  
  AudioQueryResult rslt = m_db->getViaFilter(filt);
  result = rslt.toJson();
  
  return true;
}
