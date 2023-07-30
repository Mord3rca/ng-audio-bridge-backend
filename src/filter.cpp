#include "filter.hpp"

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

void AudioBridgeFilter::set(const Pistache::Rest::Request &req)
{
  Json::CharReaderBuilder builder; Json::Value root;
  auto arg = req.query().get("filterJSON");
  if(!arg)
    return;

  auto jsonstr = arg.value();
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
      
      enum genre gr = strToGenre( _g );
      m_allowedgenre.push_back( gr );
    }
  }
}

bool AudioBridgeFilter::validate() const noexcept
{
  if( !std::regex_match(m_mindate, regdate) ) return false;
  
  if( !m_maxdate.empty() )
    if( !std::regex_match(m_maxdate, regdate) ) return false;
  
  return true;
}

const std::string AudioBridgeFilter::getQuery() const noexcept
{
  std::ostringstream sql_query; std::vector<std::string> conditions;
  //sql_query << "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks";
  sql_query << "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id IN (SELECT id FROM Tracks";
  
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
  
  sql_query << " ORDER BY RANDOM() LIMIT 25);";
  
  //std::cout << "Executed query: " << sql_query.str() << std::endl;
  
  return sql_query.str();
}

APIFilter::APIFilter() :  m_mindate("2003/01/01"), m_maxdate(""),
                          m_minscore(0), m_maxscore(5),
                          m_allowUnrated(true)
{
  m_allowedgenre = default_allowed_genre;
}

APIFilter::~APIFilter(){}

void APIFilter::set(const Pistache::Rest::Request &req)
{
  if( req.hasParam("filterObject") )
    set_json(req);
  else
    set_post(req);
}

void APIFilter::set_json(const Pistache::Rest::Request &req)
{
  Json::CharReaderBuilder builder; Json::CharReader *reader = builder.newCharReader();
  std::string json_err; Json::Value root;
  const char *stream = req.param("filterObject").as<std::string>().c_str();
  
  if( !reader->parse( stream, stream + req.param("filterObject").as<std::string>().length(),
                      &root, &json_err) )
  {
    delete reader;
    return;
  }
  delete reader;
  
  if( root["minDate"] != Json::Value::nullSingleton() && root["minDate"].isString() )
    m_mindate = root["minDate"].asString();
  
  if( root["maxDate"] != Json::Value::nullSingleton() && root["maxDate"].isString() )
    m_maxdate = root["maxDate"].asString();
  
  if( root["minScore"] != Json::Value::nullSingleton() && root["minScore"].isNumeric() )
    m_minscore = root["minScore"].asFloat();
  
  if( root["maxScore"] != Json::Value::nullSingleton() && root["maxScore"].isNumeric() )
    m_maxscore = root["maxScore"].asFloat();
  
  if( m_minscore > m_maxscore )
  {
    float tmp = m_maxscore;
    m_maxscore = m_minscore;
    m_minscore = tmp;
  }
  
  if( root["allowUnrated"] != Json::Value::nullSingleton() && root["allowUnrated"].isBool() )
    m_allowUnrated = root["allowUnrated"].asBool();
  
  _read_genres_array(root);
}

void APIFilter::set_post(const Pistache::Rest::Request &req)
{
  if( req.hasParam("minDate") )
    m_mindate = req.param("minDate").as<std::string>();
  
  if( req.hasParam("maxDate") )
    m_maxdate = req.param("maxDate").as<std::string>();
  
  if( req.hasParam("minScore") )
    m_minscore = req.param("minScore").as<float>();
  
  if( req.hasParam("maxScore") )
    m_maxscore = req.param("maxScore").as<float>();
  
  if( m_minscore > m_maxscore )
  {
    float tmp = m_maxscore;
    m_maxscore = m_minscore;
    m_minscore = tmp;
  }
  
  if( req.hasParam("allowUnrated") )
    m_allowUnrated = req.param("allowUnrated").as<bool>();
  
  if( !req.hasParam("allowedGenre") ) return;
  
  std::string allowedgenre_str = req.param("allowedGenre").as<std::string>(), err;
  Json::CharReaderBuilder builder; Json::CharReader *reader = builder.newCharReader();
  Json::Value root;
  
  if( !reader->parse(allowedgenre_str.c_str(), allowedgenre_str.c_str() + allowedgenre_str.length(),
      &root, &err) )
  {
    delete reader; return;
  }
  delete reader;
  
  _read_genres_array(root);
}

void APIFilter::_read_genres_array( const Json::Value &root )
{
  if( !root.empty() && root["genres"].isArray() )
  {
    m_allowedgenre.clear();
    for( auto i : root["genres"] )
      if( i.isInt() )
      {
        try
        {
          m_allowedgenre.push_back( static_cast<enum genre>( i.asInt() ) );
        }
        catch( std::exception& ){continue;}
      }
  }
}

bool APIFilter::validate() const noexcept
{
  //Date check.
  if( !std::regex_match( m_mindate, regdate ) ) return false;

  if( !m_maxdate.empty() )
    if( !std::regex_match(m_maxdate, regdate) ) return false;

  if( !(m_minscore >= 0 && m_minscore <= 5) || !( m_maxscore >= 0 && m_maxscore <= 5 ) )
    return false;
  
  return true;
}

const std::string APIFilter::getQuery() const noexcept
{
  std::ostringstream sql_query; std::vector<std::string> conditions;
  sql_query << "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id IN (SELECT id FROM Tracks";
  
  if( m_minscore != 0 || m_maxscore != 5 )
  {
    std::string tmp = "(score BETWEEN " + std::to_string(m_minscore) + " AND " + std::to_string(m_maxscore) + ")";
    
    if( m_allowUnrated )
      tmp = "( " + tmp + "OR score=-1)";
    
    conditions.push_back( tmp );
  }
  
  if( m_mindate != "2003/01/01" || !m_maxdate.empty() )
    conditions.push_back("(submission_date BETWEEN \"" + m_mindate + "\" AND \"" + m_maxdate + "\")" );
  
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
  
  sql_query << " ORDER BY RANDOM() LIMIT 25);";
  
  return sql_query.str();
}

APIFilterComposer::APIFilterComposer(){}

APIFilterComposer::~APIFilterComposer(){}

void APIFilterComposer::set(const Pistache::Rest::Request &req)
{
  m_composer = req.param("composer").as<std::string>();
}

bool APIFilterComposer::validate() const noexcept
{
  return !m_composer.empty();
}

const std::string APIFilterComposer::getQuery() const noexcept
{
  std::string result; char *buff;
  buff = sqlite3_mprintf("SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id IN (SELECT id FROM Tracks WHERE composer LIKE \"%q\");", m_composer.c_str());
  result = buff;
  sqlite3_free(buff);
  return result;
}

void APIFilterRange::set(const Pistache::Rest::Request &req)
{
  unsigned int page = 0, mul = 100;
  auto page_str = req.param("page").as<std::string>();
  auto mul_str  = req.param("num").as<std::string>();
  
  if( std::regex_match(mul_str, regnum) )
    mul = std::atoi( mul_str.c_str() );
  
  if( std::regex_match(page_str, regnum) )
  {
    page = std::atoi( page_str.c_str() );
    
    m_min = page*mul + 1;
    m_max = (page+1)*mul;
  }
}
