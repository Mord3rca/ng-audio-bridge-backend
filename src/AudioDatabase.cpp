#include "AudioDatabase.hpp"

AudioDatabase::AudioDatabase() : m_handler(nullptr), m_path("")
{}

AudioDatabase::AudioDatabase( const std::string& filename )
{
  if( !openDBFile(filename))
    throw std::runtime_error( "Can't open sqlite3 DB."  );
  
  m_path = filename;
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

void AudioDatabase::reload()
{
  if( m_handler ) sqlite3_close_v2(m_handler);
  
  this->openDBFile(m_path);
}

const AudioQueryResult AudioDatabase::getSongByID( const unsigned int id )
{
  std::string query = "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id = " + std::to_string(id) + ";";
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
  sql_query << "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks";
  
  if( f.getMinDate() != "2003/01/01" )
    conditions.push_back( "(submission_date BETWEEN \"" + f.getMinDate() + "\" AND \"" + f.getMaxDate() +"\")");
  
  if( f.getMinScore() != 0 || f.getMaxScore() != 5 )
    conditions.push_back( "(score BETWEEN " + std::to_string( f.getMinScore() ) + " AND " + std::to_string( f.getMaxScore() ) + ")");
  
  if( f.getAllowedGenre().size() < 48 )
  {
    auto genres = f.getAllowedGenre();
    if( genres.size() == 1 )
      conditions.push_back("genre=" + std::to_string( static_cast<int>(genres[0]) ));
    else
    {
      std::string tmp = "genre IN (";
      for(size_t i = 0; i < genres.size(); i++)
      {
        tmp += std::to_string( static_cast<int>(genres[i]) );
        if( i != genres.size() - 1 )
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
  sqlite3_exec( m_handler, "CREATE INDEX Tracks_Index ON Tracks(score, submission_date, genre);",
                nullptr, nullptr, nullptr);
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
