#ifndef SRC_AUDIODATABASE_HPP_
#define SRC_AUDIODATABASE_HPP_

extern "C" {
  #include <sqlite3.h>
}

#include <map>
#include <vector>
#include <string>

#include "songItem.hpp"
#include "filter.hpp"

class AudioQueryResult {
    friend class AudioDatabase;
 public:
    AudioQueryResult() {}
    ~AudioQueryResult() {
        for (auto i : m_songs)
            delete i;
    }

    bool isEmpty() const
    {return m_songs.empty();}
    const SongItem& operator[](ssize_t i) const
    {return *m_songs[i];}

    const std::vector<const SongItem*> songs() const
    {return m_songs;}

 private:
    std::vector<const SongItem*> m_songs;
};

class AudioDatabase {
 public:
    AudioDatabase();
    explicit AudioDatabase(const std::string&);
    ~AudioDatabase();

    bool openDBFile(const std::string&, bool = false);
    void reload();

    const AudioQueryResult getSongByID(const unsigned int);
    const AudioQueryResult getSongsByName(const std::string&, size_t&);

    const AudioQueryResult getViaFilter(const IFilter&);

    const std::map<int, std::string> getGenreList();

    unsigned int getMaxId() const noexcept
    {return m_info.max_id;}

    unsigned int getTracksCount() const noexcept
    {return m_info.count;}

    // Write method
    bool createSong(const SongItem&, const std::vector<std::string>&);
    bool deleteSong(const SongItem&);
    bool updateSong(const SongItem&, const std::vector<std::string>&);

 protected:
    static int sqlite3_callback(void*, int, char**, char**);
    static int sqlite3_info_callback(void*, int, char**, char**);
    static int sqlite3_genre_callback(void*, int, char**, char**);

 private:
    int _loadDBInMemory(const std::string&);
    void _createIndex();
    void _getDBInfo();
    sqlite3* m_handler; std::string m_path;

    struct {
        unsigned int max_id;
        unsigned int count;
    } m_info;
};

#endif  // SRC_AUDIODATABASE_HPP_
