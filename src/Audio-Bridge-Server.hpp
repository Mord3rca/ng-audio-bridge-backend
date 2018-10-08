#ifndef AUDIO_BRIDGE_SERVER_HPP
#define AUDIO_BRIDGE_SERVER_HPP

#include <http/worker>

#include "AudioDatabase.hpp"
#include "filter.hpp"

#include <json/json.h>

class AudioServer : public http::Worker
{
  static const std::string m_crossdomain;
public:
  AudioServer();
  AudioServer(const std::string&, const unsigned int);
  
  ~AudioServer();
  
  void setDBController( AudioDatabase* );
  AudioDatabase* getDBController();
  
  void OnGet(http::Client&, const http::Request&) override;
  void OnPost(http::Client&, const http::Request&) override;
  
protected:
  //Old API compatibility
  void _audiobridge_process (http::Client&, const http::Request&);
  void _audiobridge_getmp3  (http::Client&, const http::Request&);
  
  //New API Block
  //POST REQUEST
  void _api_filter(http::Client&, const http::Request&);
  void _api_filter_composer(http::Client&, const http::Request&);
  
  //GET Request
  void _api_version(http::Client&);
  void _api_list(http::Client&, const http::Request&);
  void _api_track_id(http::Client&, const http::Request&);
  void _api_track_random(http::Client&, const http::Request&);
  void _api_genrelist(http::Client&, const http::Request&);
  
private:
  AudioDatabase *m_db;
};

#endif //AUDIO_BRIDGE_SERVER_HPP
