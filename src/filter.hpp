#ifndef FILTER_HPP
#define FILTER_HPP

#include <regex>
#include <string>
#include <sstream>

#include <json/json.h>

#include <sqlite3.h>

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

class APIFilter : public IFilter
{
public:
  APIFilter();
  ~APIFilter();
  
  void set(const http::Request&);
  bool validate() const noexcept;
  
  const std::string getQuery() const noexcept;
  
protected:
  void set_json(const http::Request&);
  void set_post(const http::Request&);
private:
  void _read_genres_array( const Json::Value& );
  std::string m_mindate, m_maxdate;
  float m_minscore, m_maxscore;
  bool m_allowUnrated;
  
  std::vector<enum genre> m_allowedgenre;
};

class APIFilterComposer : public IFilter
{
public:
  APIFilterComposer();
  ~APIFilterComposer();
  
  void set(const http::Request&);
  bool validate() const noexcept;
  
  const std::string getQuery() const noexcept;
  
private:
  std::string m_composer;
};

static const std::regex regdate("^\\d{4}/\\d{2}/\\d{2}$");

#endif //FILTER_HPP
