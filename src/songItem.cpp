#include "songItem.hpp"

static std::map< enum genre, const std::string > _genreStr = {{
  {genre::CLASSICAL, "Classical"}, {genre::JAZZ, "Jazz"},
  {genre::SOLO_INSTRUMENT, "Solo Instrument"},
  
  {genre::AMBIENT, "Ambient"}, {genre::CHIPSTEP, "Chipstep"},
  {genre::DANCE, "Dance"}, {genre::DRUM_N_BASS, "Drum N Bass"},
  {genre::DUPSTEP, "Dupstep"}, {genre::HOUSE, "House"},
  {genre::INDUSTRIAL, "Industrial"}, {genre::NEW_WAVE, "New Wave"},
  {genre::SYNTHWAVE, "Synthwave"}, {genre::TECHNO, "Techno"},
  {genre::TRANCE, "Trance"}, {genre::VIDEO_GAME, "Video Game"},
  
  {genre::HIP_HOP_MODERN, "Hip Hop - Modern"},
  {genre::HIP_HOP_OLSKOOL, "Hip Hop - Olskool"},
  {genre::NERDCORE, "Nerdcore"}, {genre::R_B, "R&B"},
  
  {genre::BRIT_POP, "Brit Pop"}, {genre::CLASSICAL_ROCK, "Classical Rock"},
  {genre::GENERAL_ROCK, "General Rock"}, {genre::GRUNGE, "Grunge"},
  {genre::HEAVY_METAL, "Heavy Metal"}, {genre::INDIE, "Indie"},
  {genre::POP, "Pop"}, {genre::PUNK, "Punk"},
  
  {genre::CINEMATIC, "Cinematic"}, {genre::EXPERIMENTAL, "Experimental"},
  {genre::FUNK, "Funk"}, {genre::FUSION, "Fusion"}, {genre::GOTH, "Goth"},
  {genre::MISCELLANEOUS, "Miscellaneous"}, {genre::SKA, "Ska"},
  {genre::WORLD, "world"},
  
  {genre::DISCUSSION, "Discussion"}, {genre::MUSIC, "Music"},
  {genre::STORYTELLING, "StoryTelling"},
  
  {genre::BLUEGRASS, "Bluegrass"}, {genre::BLUES, "Blues"},
  {genre::COUNTRY, "Country"},
  
  {genre::A_CAPELLA, "A Capella"}, {genre::COMEDY, "Comedy"},
  {genre::CREEPYPASTA, "Creepypasta"}, {genre::DRAMA, "Drama"},
  {genre::INFORMATIONAL, "Informational"}, {genre::SPOKEN_WORLD, "Spoken World"},
  {genre::VOICE_DEMO, "Voice Demo"},
  
  {genre::UNKNOWN, "Genre Unknown"}
}};

static std::array< const std::string, 9 > _genreGroupStr = {{
 "Easy Listening", "Electronic", "Hip Hop, Rap, R&B",
 "Metal, Rock", "Other", "Podcasts", "Southern Flavor",
 "Voice Acting",
 
 //Joker
 "Group Genre Unknown"
}};

static const std::string& genreToStr(enum genre& _g)
{
  return _genreStr[_g];
}

static const std::string& genreToGroupStr(enum genre& _g)
{
  switch( _g )
  {
    case genre::CLASSICAL:
    case genre::JAZZ:
    case genre::SOLO_INSTRUMENT:
      return _genreGroupStr[0];
    
    case genre::AMBIENT:
    case genre::CHIPSTEP:
    case genre::DANCE:
    case genre::DRUM_N_BASS:
    case genre::DUPSTEP:
    case genre::HOUSE:
    case genre::INDUSTRIAL:
    case genre::NEW_WAVE:
    case genre::SYNTHWAVE:
    case genre::TECHNO:
    case genre::TRANCE:
    case genre::VIDEO_GAME:
      return _genreGroupStr[1];
    
    case genre::HIP_HOP_OLSKOOL:
    case genre::HIP_HOP_MODERN:
    case genre::NERDCORE:
    case genre::R_B:
      return _genreGroupStr[2];
    
    case genre::BRIT_POP:
    case genre::CLASSICAL_ROCK:
    case genre::GENERAL_ROCK:
    case genre::GRUNGE:
    case genre::HEAVY_METAL:
    case genre::INDIE:
    case genre::POP:
    case genre::PUNK:
      return _genreGroupStr[3];
    
    case genre::CINEMATIC:
    case genre::EXPERIMENTAL:
    case genre::FUNK:
    case genre::FUSION:
    case genre::GOTH:
    case genre::MISCELLANEOUS:
    case genre::SKA:
    case genre::WORLD:
      return _genreGroupStr[4];
    
    case genre::DISCUSSION:
    case genre::MUSIC:
    case genre::STORYTELLING:
      return _genreGroupStr[5];
    
    case genre::BLUEGRASS:
    case genre::BLUES:
    case genre::COUNTRY:
      return _genreGroupStr[6];
    
    case genre::A_CAPELLA:
    case genre::COMEDY:
    case genre::CREEPYPASTA:
    case genre::DRAMA:
    case genre::INFORMATIONAL:
    case genre::SPOKEN_WORLD:
    case genre::VOICE_DEMO:
      return _genreGroupStr[7];
    
    case genre::UNKNOWN:
    default:
      return _genreGroupStr[8];
  }
}

SongItem::SongItem() : m_id(0), m_genre(genre::UNKNOWN), m_score(-1)
{}

SongItem::SongItem(int argc, char **argv, char **azColumn) : SongItem()
{
  int count = argc;
  while( count-- )
  {
    std::string columnName = azColumn[count], value = (argv[count] != nullptr ? argv[count] : "");
    
    if( columnName == "Id" )
      m_id = std::stoul( value );
    else if( columnName == "title" )
      m_title = value;
    else if( columnName == "composer" )
      m_composerName = value;
    else if( columnName == "score" )
      m_score = std::stof(value);
    else if( columnName == "date" )
      m_date = value;
    else if( columnName == "genre" )
      m_genre = genre::UNKNOWN;
    else if( columnName == "url" )
      m_path = value;
  }
}

SongItem::~SongItem()
{}

bool SongItem::operator==( const SongItem& s ) const noexcept
{
  return s.getId() == this->getId();
}
