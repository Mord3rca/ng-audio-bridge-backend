#include "filter.hpp"

const std::regex AudioBridgeFilter::m_regdate("^\\d{4}/\\d{2}/\\d{2}$");

static std::vector<enum genre> default_allowed_genre = {
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

AudioBridgeFilter::AudioBridgeFilter() :  m_minscore(0), m_maxscore(5),
                                          m_mindate("2003/01/01"),
                                          m_allowedgenre( default_allowed_genre )
{}

AudioBridgeFilter::~AudioBridgeFilter()
{}

void AudioBridgeFilter::set( const http::Request &req )
{
  
  Json::CharReaderBuilder builder; Json::Value root;
  std::string jsonstr = req.getVariable("filterJSON");
  if( jsonstr.empty() )
    return;

  Json::CharReader *json_read = builder.newCharReader();
  std::string err;

  if( !json_read->parse( jsonstr.c_str(), jsonstr.c_str() + jsonstr.length() , &root, &err) )
  {
    delete json_read;
    return;
  }
  
  delete json_read;
  
  if( root["minScore"] != Json::Value::nullSingleton() )
    m_minscore = root["minScore"].asFloat();
  
  if( root["maxScore"] != Json::Value::nullSingleton() )
    m_maxscore = root["maxScore"].asFloat();
  
  std::string::size_type pos;
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

bool AudioBridgeFilter::validate() const noexcept
{
  if( !std::regex_match(m_mindate, m_regdate) ) return false;
  
  if( !m_maxdate.empty() )
    if( !std::regex_match(m_maxdate, m_regdate) ) return false;
  
  return true;
}

const std::string AudioBridgeFilter::getQuery() const noexcept
{
  std::ostringstream sql_query; std::vector<std::string> conditions;
  sql_query << "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks";
  
  if( !m_maxdate.empty() && m_mindate != "2003/01/01" )
    conditions.push_back( "(submission_date BETWEEN \"" + m_mindate + "\" AND \"" + m_maxdate +"\")");
  
  if( m_minscore != 0 || m_maxscore != 5 )
    conditions.push_back( "(score BETWEEN " + std::to_string( m_minscore ) + " AND " + std::to_string( m_maxscore ) + ")");
  
  if( m_allowedgenre.size() < 48 )
  {
    if( m_allowedgenre.size() == 1 )
      conditions.push_back("genre=" + std::to_string( static_cast<int>(m_allowedgenre[0]) ));
    else
    {
      std::string tmp = "genre IN (";
      for(size_t i = 0; i < m_allowedgenre.size(); i++)
      {
        tmp += std::to_string( static_cast<int>(m_allowedgenre[i]) );
        if( i != m_allowedgenre.size() - 1 )
          tmp += ", ";
      }
      tmp+=')';
      conditions.push_back(tmp);
    }
        
  }
  
  if( !conditions.empty() )
    sql_query << " WHERE ";
  
  for(unsigned int i = 0; i < conditions.size(); i++)
  {
    sql_query << conditions[i];
    if( i != conditions.size() -1)
      sql_query << " AND ";
  }
  
  sql_query << " ORDER BY RANDOM() LIMIT 25;";
  
  //std::cout << "Executed query: " << sql_query.str() << std::endl;
  
  return sql_query.str();
}
