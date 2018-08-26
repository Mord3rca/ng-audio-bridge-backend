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

class APIFilterRandom : public IFilter
{
public:
  APIFilterRandom(){}
  ~APIFilterRandom(){}
  
  void set( const http::Request& ){}
  bool validate() const noexcept { return true; }
  
  const std::string getQuery() const noexcept
  { return "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks ORDER BY RANDOM() LIMIT 1;"; }
};

class APIFilterRange : public IFilter
{
public:
  APIFilterRange() : m_min(0), m_max(0) {}
  ~APIFilterRange(){}
  
  void set( const http::Request& );
  bool validate() const noexcept
  { return m_min != 0 && m_max != 0;}
  
  const std::string getQuery() const noexcept
  { return  "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id BETWEEN "
            + std::to_string(m_min) + " AND " + std::to_string(m_max) +";"; }

private:
  unsigned int m_min, m_max;
};

static const std::regex regdate ("^\\d{4}/\\d{2}/\\d{2}$");
static const std::regex regnum  ("\\d*");

#endif //FILTER_HPP
