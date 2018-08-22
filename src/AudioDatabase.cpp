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

const AudioQueryResult AudioDatabase::getViaFilter( const IFilter &f )
{
  std::string query = f.getQuery();
  
  AudioQueryResult rslt;
  sqlite3_exec(m_handler, query.c_str(), &AudioDatabase::sqlite3_callback, &rslt, NULL);
  
  return rslt;
}

const std::map<int, std::string> AudioDatabase::getGenreList()
{
  std::map<int, std::string> result;
  sqlite3_exec(m_handler, "SELECT * FROM GenreType;", &AudioDatabase::sqlite3_genre_callback, &result, nullptr);
  
  return result;
}

int AudioDatabase::sqlite3_callback(void* data, int argc, char **argv, char **azColumn)
{
  AudioQueryResult *obj = static_cast<AudioQueryResult*>(data);
  
  obj->m_songs.push_back( new SongItem(argc, argv, azColumn) );
  
  return 0;
}

int AudioDatabase::sqlite3_genre_callback(void* data, int argc, char **argv, char **azColumn)
{
  auto obj = static_cast< std::map<int, std::string>* >(data);
  int i; std::string j;
  i = std::atoi( argv[0] );
  j = argv[1];
  
  (*obj)[i] = j;
  
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
  sqlite3_exec( m_handler, "CREATE INDEX Tracks_Index ON Tracks(score, submission_date, genre, composer);",
                nullptr, nullptr, nullptr);
}

AudioQueryResult::AudioQueryResult(){}

AudioQueryResult::~AudioQueryResult()
{
  for(auto i : m_songs)
    delete i;
}

static const std::string JSONSafe( const std::string &e)
{
  std::string result = e; std::string::size_type pos = 0;
  if( result[0] == '"' )
    result.replace(0,1,"\\\"");
    
  while( ( pos = result.find("\"", pos+2 ) ) != std::string::npos )
    result.replace(pos, pos+1, "\\\"");
  
  return result;
}

const std::string AudioQueryResult::toJson() const
{
  std::ostringstream rslt;
  
  rslt << '[';
  for(unsigned int i = 0; i < m_songs.size(); i++)
  {
    const SongItem *snd = m_songs[i];
    rslt  << "{\"id\":" << snd->getId()
          << ",\"composer\":\"" << JSONSafe( snd->getComposerName() ) << "\""
          << ",\"title\":\"" << JSONSafe( snd->getSongName() ) << "\""
          << ",\"score\":" << snd->getScore()
          << ",\"genre\":\"" << genreToStr( snd->getGenre() ) << "\""
          << ",\"date\":\"" << snd->getSubmissionDate() << "\"}";
    if( i != m_songs.size() -1 )
      rslt << ", ";
  }
  rslt << ']';
  
  return rslt.str();
}
