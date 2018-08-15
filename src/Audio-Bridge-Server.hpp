#ifndef AUDIO_BRIDGE_SERVER_HPP
#define AUDIO_BRIDGE_SERVER_HPP

#include "HTTPServer.hpp"
#include "AudioDatabase.hpp"
#include "filter.hpp"

#include <json/json.h>

class AudioServer : public http::Server
{
public:
  AudioServer();
  AudioServer(const std::string&, const unsigned int);
  
  ~AudioServer();
  
  void setDBController( AudioDatabase* );
  AudioDatabase* getDBController();
  
  void OnGet(http::Client&, const http::Request&) override;
  void OnPost(http::Client&, const http::Request&) override;
  
private:
  void _audiobridge_sendCD  (http::Client&);
  void _audiobridge_process (http::Client&, const http::Request&);
  void _audiobridge_getmp3  (http::Client&, const http::Request&);
  bool _audiobridge_JSONprocess(const http::Request&, std::string&);
  
  AudioDatabase *m_db;
};

#endif //AUDIO_BRIDGE_SERVER_HPP
