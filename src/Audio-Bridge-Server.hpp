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
  bool _process(const http::Request&, std::string&);
  AudioDatabase *m_db;
};

#endif //AUDIO_BRIDGE_SERVER_HPP
