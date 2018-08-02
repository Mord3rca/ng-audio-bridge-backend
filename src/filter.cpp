#include "filter.hpp"

constexpr enum genre ALL_GENRE[] {genre::AMBIENT};

filter::filter() :  m_minscore(0), m_maxscore(5),
                    m_mindate("2003/01/01")
{
  for(auto i : ALL_GENRE)
    m_allowedgenre.push_back(i);
}

filter::filter( const Json::Value& root ) : filter()
{
  setViaAudioBridgeJson(root);
}

filter::~filter()
{}

void filter::setViaAudioBridgeJson( const Json::Value& root )
{
  try{
    m_minscore = root["minScore"].asFloat();
    m_maxscore = root["maxScore"].asFloat();
    m_mindate = root["minDate"].asString();
    m_maxdate = root["maxDate"].asString();
  } catch( std::exception& err )
  {}
  
  std::string::size_type pos = 0;
  while( (pos = m_mindate.find('-')) != std::string::npos )
    m_mindate.replace(pos, 1, "/");
  while( (pos = m_maxdate.find('-')) != std::string::npos )
    m_maxdate.replace(pos, 1, "/");
}
