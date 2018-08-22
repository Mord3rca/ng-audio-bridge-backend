#include "Audio-Bridge-Server.hpp"

const std::string AudioServer::m_crossdomain =
"<?xml version=\"1.0\"?>\
<!DOCTYPE cross-domain-policy SYSTEM \"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">\
<cross-domain-policy>\
    <site-control permitted-cross-domain-policies=\"master-only\"/>\
    <allow-access-from domain=\"*\" secure=\"false\"/>\
    <allow-http-request-headers-from domain=\"*\" headers=\"*\"/>\
</cross-domain-policy>";

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
  
  if( request == "/Radio2/FilterBridge.php" || request == "/api/filter/old" )
    _audiobridge_process(client, req);
  else if( request == "/api/filter/" )
    _api_filter( client, req );
  else if( request == "/api/filter/composer" )
    _api_filter_composer( client, req );
  else
    client << http::genericAnswer[ 2 ]; // Forbidden
}

void AudioServer::OnGet( http::Client &client, const http::Request &req)
{
  if( req.getPath() == "/crossdomain.xml" )
  {
    http::Response resp;
    resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Content-Type", "text/xml");
    
    resp.appendData(m_crossdomain);
    
    client << resp;
  }
  else if( req.getPath().rfind(".mp3") != std::string::npos )
    _audiobridge_getmp3(client, req);
  else if( req.getPath().find("/api/track/") != std::string::npos )
    _api_track_id(client, req);
  else if( req.getPath() == "/api/version" )
    _api_version(client);
  else if( req.getPath() == "/api/genres" )
    _api_genrelist(client, req);
  else
    client << http::genericAnswer[ 1 ]; //Not Found.
}

void AudioServer::_audiobridge_process(http::Client &client, const http::Request &req)
{
  if(!m_db) { client << http::genericAnswer[3]; return; }
  
  AudioBridgeFilter filter; filter.set(req);
  
  if( filter.validate() )
  {
    AudioQueryResult rslt = m_db->getViaFilter(filter);
    
    http::Response resp;
    resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.addHeader("Content-Type", "application/json");
    
    resp.appendData("{\"ResultSet\":");
    resp.appendData(rslt.toJson());
    resp.appendData("}");
    
    client << resp;
  }
  else
    client << http::genericAnswer[3]; // SERVER ERROR
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
    }
  }
  else
  {
    resp.setStatusCode( http::status_code::INTERNAL_SERVER_ERROR );
    resp.addHeader("Content-Type", "text/html");
    resp.appendData("Song Index unavailable.");
  }
}

//POST
void AudioServer::_api_filter(http::Client &client, const http::Request &req)
{
  if(!m_db) { client << http::genericAnswer[3]; return; }
  
  APIFilter filter; filter.set(req);
  
  if( filter.validate() )
  {
    AudioQueryResult rslt = m_db->getViaFilter(filter);
    
    http::Response resp; resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.addHeader("Content-Type","application/json");
    resp.appendData("{\"Tracks\":");
    resp.appendData( rslt.toJson() );
    resp.appendData("}");
    
    client << resp;
  }
  else
    client << http::genericAnswer[3];
}

void AudioServer::_api_filter_composer(http::Client &client, const http::Request &req)
{
  if(!m_db) { client << http::genericAnswer[3]; return; }
  
  APIFilterComposer filter; filter.set(req);
  
  if( filter.validate() )
  {
    AudioQueryResult rslt = m_db->getViaFilter(filter);
    
    http::Response resp; resp.setStatusCode(http::status_code::OK);
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.addHeader("Content-Type","application/json");
    resp.appendData("{\"Tracks\":");
    resp.appendData( rslt.toJson() );
    resp.appendData("}");
    
    client << resp;
  }
  else
    client << http::genericAnswer[3];
}

//GET
void AudioServer::_api_version(http::Client &client)
{
  http::Response resp;
  resp.setStatusCode( http::status_code::OK );
  resp.addHeader("Access-Control-Allow-Origin", "*");
  resp.addHeader("Content-Type", "text/plain");
  resp.appendData("Version: 0.1");
  
  client << resp;
}

void AudioServer::_api_track_id(http::Client &client, const http::Request &req)
{
  static std::regex number_only("\\d*");
  if(!m_db) { client << http::genericAnswer[3]; return; }
  
  //Schema: /api/track/<ID>
  //Get ID from path requested
  std::istringstream stream( req.getPath() );
  std::string str_id;
  for( int i = 4; i > 0; i-- )
    std::getline(stream, str_id, '/');
  
  if( !std::regex_match(str_id, number_only) )
  { client << http::genericAnswer[ 3 ]; return; }
  
  unsigned int id = std::strtoul( str_id.c_str(), nullptr, 10 );
  
  //ID OK: Processing query
  AudioQueryResult rslt = m_db->getSongByID( id );
  if( rslt.isEmpty() )
    client << http::genericAnswer[1];
  else
  {
    http::Response resp;
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.setStatusCode( http::status_code::MOVED_PERMANENTLY );
    resp.addHeader("Location", rslt[0].getURL() );
    client << resp;
  }
}

void AudioServer::_api_genrelist(http::Client &client, const http::Request &req)
{
  if(!m_db) { client << http::genericAnswer[3]; return; }
  
  auto i = m_db->getGenreList();
  
  http::Response resp;
  resp.setStatusCode(http::status_code::OK);
  if( req.isVarExist("json") )
  {
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.addHeader("Content-Type","application/json");
    
    resp.appendData("{\"genres\":[");
    for(auto j : i)
      resp.appendData( "{\"id\":" + std::to_string( std::get<0>(j) ) + ", \"name\":\"" + std::get<1>(j) + "\"}, " );
    resp.appendData("{} ]}");
  }
  else
  {
    resp.addHeader("Access-Control-Allow-Origin", "*");
    resp.addHeader("Content-Type","text/plain");
    for( auto j : i )
      resp.appendData( std::get<1>(j) + ": " + std::to_string( std::get<0>(j) ) + "\n");
  }
  client << resp;
}
