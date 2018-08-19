#include "filter.hpp"

filter::filter() :  m_minscore(0), m_maxscore(5),
                    m_mindate("2003/01/01")
{
  m_allowedgenre = {
    genre::CLASSICAL, genre::JAZZ, genre::SOLO_INSTRUMENT,
  
    genre::AMBIENT, genre::CHIPSTEP, genre::DANCE, genre::DRUM_N_BASS, genre::DUBSTEP,
    genre::HOUSE, genre::INDUSTRIAL, genre::NEW_WAVE, genre::SYNTHWAVE, genre::TECHNO,
    genre::TRANCE, genre::VIDEO_GAME,
    
    genre::HIP_HOP_MODERN, genre::HIP_HOP_OLSKOOL, genre::NERDCORE, genre::R_B,

    genre::BRIT_POP, genre::CLASSICAL_ROCK, genre::GENERAL_ROCK, genre::GRUNGE,
    genre::HEAVY_METAL, genre::INDIE, genre::POP, genre::PUNK,

    genre::CINEMATIC, genre::EXPERIMENTAL, genre::FUNK, genre::FUSION, genre::GOTH, genre::MISCELLANEOUS,
    genre::SKA, genre::WORLD,

    genre::DISCUSSION, genre::MUSIC, genre::STORYTELLING,

    genre::BLUEGRASS, genre::BLUES, genre::COUNTRY,
    
    genre::A_CAPELLA, genre::COMEDY, genre::CREEPYPASTA, genre::DRAMA,
    genre::INFORMATIONAL, genre::SPOKEN_WORLD, genre::VOICE_DEMO
  };
}

filter::filter( const Json::Value& root ) : filter()
{
  setViaAudioBridgeJson(root);
}

filter::~filter()
{}

void filter::setViaAudioBridgeJson( const Json::Value& root )
{
  std::string::size_type pos = 0;
  
  if( root["minScore"] != Json::Value::nullSingleton() )
    m_minscore = root["minScore"].asFloat();
  
  if( root["maxScore"] != Json::Value::nullSingleton() )
    m_maxscore = root["maxScore"].asFloat();
  
  if( root["minDate"] != Json::Value::nullSingleton() )
  {
    m_mindate = root["minDate"].asString();
    while( (pos = m_mindate.find('-')) != std::string::npos )
      m_mindate.replace(pos, 1, "/");
  }
  
  if( root["maxDate"] != Json::Value::nullSingleton() )
  {
    m_maxdate = root["maxDate"].asString();
    
    while( (pos = m_maxdate.find('-')) != std::string::npos )
      m_maxdate.replace(pos, 1, "/");
  }
  
  if( root["genres"] != Json::Value::nullSingleton() && root["genres"].isArray() )
  {
    m_allowedgenre.clear();
    for( auto i : root["genres"] )
    {
      std::string _g = i.asString();
      http::unescape(_g);
      
      enum genre gr = strToGenre( _g );
      m_allowedgenre.push_back( gr );
    }
  }
}
