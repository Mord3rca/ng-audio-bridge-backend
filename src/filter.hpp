#ifndef FILTER_HPP
#define FILTER_HPP

#include <regex>
#include <string>
#include <sstream>

#include <json/json.h>

#include "songItem.hpp"
#include "HTTPServer.hpp" //http::Request

class IFilter
{
public:
  IFilter(){}
  ~IFilter(){}
  
  virtual void set(const http::Request&) = 0;
  virtual bool validate() const noexcept = 0;
  
  virtual const std::string getQuery() const noexcept = 0;
};

class AudioBridgeFilter : public IFilter
{
public:
  AudioBridgeFilter();
  ~AudioBridgeFilter();
  
  void set(const http::Request&);
  bool validate() const noexcept;
  
  const std::string getQuery() const noexcept;

private:
  float m_minscore, m_maxscore;
  std::string m_mindate, m_maxdate;
  
  std::vector<enum genre> m_allowedgenre;
};

#endif //FILTER_HPP
