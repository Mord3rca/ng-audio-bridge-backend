#ifndef AUDIO_DATABASE_HPP
#define AUDIO_DATABASE_HPP

#include <exception>
#include <vector>

#include <iostream>
#include <ostream>
#include <sstream>

extern "C"
{
  #include <sqlite3.h>
}

#include "songItem.hpp"
#include "filter.hpp"

class AudioQueryResult
{
  friend class AudioDatabase;
public:
  AudioQueryResult();
  ~AudioQueryResult();
  
  bool isEmpty()const
  {return m_songs.empty();}
  const SongItem& operator[](ssize_t i) const
  {return *m_songs[i];}
  
  const std::string toJson() const;
  
private:
  std::vector<const SongItem*> m_songs;
};

class AudioDatabase
{
public:
  AudioDatabase();
  AudioDatabase(const std::string&);
  ~AudioDatabase();
  
  bool openDBFile(const std::string&, bool = false );
  void reload();
  
  const AudioQueryResult getSongByID( const unsigned int );
  const AudioQueryResult getSongsByName( const std::string&, size_t& );
  
  const AudioQueryResult getViaFilter( const filter& );

protected:
  static int sqlite3_callback( void*, int, char**, char** );
  
private:
  int _loadDBInMemory( const std::string& );
  void _createIndex();
  const std::string _create_query_from_filter( const filter&);
  sqlite3* m_handler; std::string m_path;
};

#endif //AUDIO_DATABASE_HPP
