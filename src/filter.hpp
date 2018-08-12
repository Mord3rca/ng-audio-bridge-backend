#ifndef FILTER_HPP
#define FILTER_HPP

#include <string>
#include <sstream>

#include <json/json.h>

#include "songItem.hpp"
#include "HTTPServer.hpp" //http::unescape

class filter
{
public:
  filter();
  filter( const Json::Value& );
  ~filter();
  
  void setViaAudioBridgeJson( const Json::Value& );
  
  const float getMinScore() const noexcept
  { return m_minscore; }
  const float getMaxScore() const noexcept
  { return m_maxscore; }
  
  const std::string& getMinDate() const noexcept
  { return m_mindate; }
  const std::string& getMaxDate() const noexcept
  { return m_maxdate; }
  
  const std::vector<enum genre>& getAllowedGenre() const noexcept
  {return m_allowedgenre; }
  
private:
  float m_minscore, m_maxscore;
  std::string m_mindate, m_maxdate;
  
  std::vector<enum genre> m_allowedgenre;
};

#endif //FILTER_HPP
