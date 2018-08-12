#include "AudioDatabase.hpp"

AudioDatabase::AudioDatabase() : m_handler(nullptr)
{}

AudioDatabase::AudioDatabase( const std::string& filename )
{
  if( !openDBFile(filename))
    throw std::runtime_error( "Can't open sqlite3 DB."  );
}

AudioDatabase::~AudioDatabase()
{
  if(m_handler) sqlite3_close_v2(m_handler);
}

bool AudioDatabase::openDBFile( const std::string& filename, bool live )
{
  int err = live  ? sqlite3_open_v2( filename.c_str(), &m_handler, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr)
                  : _loadDBInMemory(filename);
  
  if( !live ) _createIndex();
  
  return err == SQLITE_OK;
}

const AudioQueryResult AudioDatabase::getSongByID( const unsigned int id )
{
  std::string query = "SELECT * FROM Songs WHERE Id = " + std::to_string(id) + ";";
  AudioQueryResult rslt;
  sqlite3_exec(m_handler, query.c_str(), &AudioDatabase::sqlite3_callback, &rslt, NULL);
  
  return rslt;
}

const AudioQueryResult AudioDatabase::getViaFilter( const filter &f)
{
  std::string query = _create_query_from_filter(f);
  
  AudioQueryResult rslt;
  sqlite3_exec(m_handler, query.c_str(), &AudioDatabase::sqlite3_callback, &rslt, NULL);
  
  return rslt;
}

const std::string AudioDatabase::_create_query_from_filter( const filter &f )
{
  std::ostringstream sql_query; std::vector<std::string> conditions;
  sql_query << "SELECT * FROM Songs";
  
  if( f.getMinDate() != "2003/01/01" )
    conditions.push_back( "(date >= \"" + f.getMinDate() + "\" AND date <= \"" + f.getMaxDate() +"\")");
  
  if( f.getMinScore() != 0 || f.getMaxScore() != 5 )
    conditions.push_back( "(score >= " + std::to_string(f.getMinScore()) + " AND score <= " + std::to_string(f.getMaxScore()) + " )");
  
  if( f.getAllowedGenre().size() < 48 )
  {
    //TODO: Write the condition genre here. (after DB reorganisation)
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

int AudioDatabase::sqlite3_callback(void* data, int argc, char **argv, char **azColumn)
{
  AudioQueryResult *obj = static_cast<AudioQueryResult*>(data);
  
  obj->m_songs.push_back( new SongItem(argc, argv, azColumn) );
  
  return 0;
}

int AudioDatabase::_loadDBInMemory( const std::string& filename )
{
  sqlite3_backup *pBackup; sqlite3 *dbFile;
  int err = sqlite3_open_v2( ":memory:", &m_handler, SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nullptr );
  if( err != SQLITE_OK ) return err;
  
  err = sqlite3_open_v2( filename.c_str(), &dbFile, SQLITE_OPEN_READONLY | SQLITE_OPEN_FULLMUTEX, nullptr);
  if(err != SQLITE_OK) return err;
  
  pBackup = sqlite3_backup_init( m_handler, "main", dbFile, "main" );
  if( pBackup )
  {
    sqlite3_backup_step(pBackup, -1); //Full copy
    sqlite3_backup_finish(pBackup);
  }
  err = sqlite3_errcode(m_handler);
  
  sqlite3_close_v2(dbFile);
  
  return err;
}

void AudioDatabase::_createIndex()
{
  std::string query = "CREATE INDEX Songs_index ON Songs(score, date, genre);";
  
  sqlite3_exec(m_handler, query.c_str(), nullptr, nullptr, nullptr);
}

AudioQueryResult::AudioQueryResult(){}

AudioQueryResult::~AudioQueryResult()
{
  for(auto i : m_songs)
    delete i;
}

const std::string AudioQueryResult::toJson() const
{
  std::ostringstream rslt;
  
  rslt << '[';
  for(unsigned int i = 0; i < m_songs.size(); i++)
  {
    const SongItem *snd = m_songs[i];
    rslt  << "{\"id\":" << snd->getId()
          << ",\"composer\":\"" << snd->getComposerName() << "\""
          << ",\"title\":\"" << snd->getSongName() << "\""
          << ",\"score\":" << snd->getScore()
          << ",\"genre\":\"" << genreToStr( snd->getGenre() ) << "\""
          << ",\"date\":\"" << snd->getSubmissionDate() << "\"}";
    if( i != m_songs.size() -1 )
      rslt << ", ";
  }
  rslt << ']';
  
  return rslt.str();
}
