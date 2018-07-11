#ifndef SONG_ITEM_HPP
#define SONG_ITEM_HPP

#include <array>
#include <map>

//Genre classification of 10/07/2018
enum class genre
{
  //Genre group: Easy Listening
  CLASSICAL, JAZZ, SOLO_INSTRUMENT,
  
  //Genre group: Electronic
  AMBIENT, CHIPSTEP, DANCE, DRUM_N_BASS, DUPSTEP,
  HOUSE, INDUSTRIAL, NEW_WAVE, SYNTHWAVE, TECHNO,
  TRANCE, VIDEO_GAME,
  
  //Genre group: Hip Hop, Rap, R&B
  HIP_HOP_MODERN, HIP_HOP_OLSKOOL, NERDCORE, R_B,
  
  //Genre group: Metal, Rock
  BRIT_POP, CLASSICAL_ROCK, GENERAL_ROCK, GRUNGE,
  HEAVY_METAL, INDIE, POP, PUNK,
  
  //Genre group: Other
  CINEMATIC, EXPERIMENTAL, FUNK, FUSION, GOTH, MISCELLANEOUS,
  SKA, WORLD,
  
  //Genre group: Southern Flavor
  DISCUSSION, MUSIC, STORYTELLING,
  
  //Genre group: Voice Acting
  BLUEGRASS, BLUES, COUNTRY,
  
  A_CAPELLA, COMEDY, CREEPYPASTA, DRAMA,
  INFORMATIONAL, SPOKEN_WORLD, VOICE_DEMO,
  
  //Joker.
  UNKNOWN
};

static const std::string& genreToStr(enum genre&);
static const std::string& genreToGroupStr(enum genre&);

class SongItem
{
public:
  SongItem();
  SongItem(int, char**, char**);
  ~SongItem();
  
  bool operator==(const SongItem&) const noexcept;
  
  const unsigned int getId() const noexcept
  {return m_id;}
  
  const std::string& getSongName() const noexcept
  {return m_title;}
  const std::string& getComposerName() const noexcept
  {return m_composerName;}
  
  const float getScore() const noexcept
  {return m_score;}
  
  const enum genre&     getGenre() const noexcept
  {return m_genre;}
  
  const std::string& getSubmissionDate() const noexcept
  {return m_date;}
  
private:
  unsigned int  m_id;
  enum genre    m_genre;
  float         m_score;
  std::string   m_composerName, m_title, m_date, m_path;
};



#endif //SONG_ITEM_HPP
