#ifndef SRC_SONGITEM_HPP_
#define SRC_SONGITEM_HPP_

#include <string>
#include <ostream>

// Genre classification of 10/07/2018
enum class genre {
    // Genre group: Easy Listening
    CLASSICAL = 0, JAZZ = 1, SOLO_INSTRUMENT = 2,

    // Genre group: Electronic
    AMBIENT, CHIPSTEP, DANCE, DRUM_N_BASS, DUBSTEP,
    HOUSE, INDUSTRIAL, NEW_WAVE, SYNTHWAVE, TECHNO,
    TRANCE, VIDEO_GAME,

    // Genre group: Hip Hop, Rap, R&B
    HIP_HOP_MODERN, HIP_HOP_OLSKOOL, NERDCORE, R_B,

    // Genre group: Metal, Rock
    BRIT_POP, CLASSICAL_ROCK, GENERAL_ROCK, GRUNGE,
    HEAVY_METAL, INDIE, POP, PUNK,

    // Genre group: Other
    CINEMATIC, EXPERIMENTAL, FUNK, FUSION, GOTH, MISCELLANEOUS,
    SKA, WORLD,

    // Genre group: Southern Flavor
    DISCUSSION, MUSIC, STORYTELLING,

    // Genre group: Voice Acting
    BLUEGRASS, BLUES, COUNTRY,

    A_CAPELLA, COMEDY, CREEPYPASTA, DRAMA,
    INFORMATIONAL, SPOKEN_WORLD, VOICE_DEMO,

    // Joker.
    UNKNOWN = -1
};

const std::string& genreToStr(const enum genre&);
const std::string& genreToGroupStr(const enum genre&);
const enum genre strToGenre(const std::string&);

class SongItem {
 public:
    SongItem();
    SongItem(int, char**, char**);

    bool operator==(const SongItem& s) const noexcept
    {return s.id() == this->id();}

    void id(unsigned int id)
    {m_id = id;}
    const unsigned int id() const noexcept
    {return m_id;}

    void title(const std::string &title)
    {m_title = title;}
    const std::string& title() const noexcept
    {return m_title;}

    void composer(const std::string &composer)
    {m_composerName = composer;}
    const std::string& composer() const noexcept
    {return m_composerName;}

    void score(const float score)
    {m_score = score;}
    const float score() const noexcept
    {return m_score;}

    void genre(const enum genre genre)
    {m_genre = genre;}
    void genre(const std::string &genre)
    {m_genre = strToGenre(genre);}
    const enum genre& genre() const noexcept
    {return m_genre;}

    void date(const std::string &date)
    {m_date = date;}
    const std::string& date() const noexcept
    {return m_date;}

    void url(const std::string &url)
    {m_path = url;}
    const std::string& url() const noexcept
    {return m_path;}

    friend std::ostream& operator<<(std::ostream&, const SongItem&);

 private:
    unsigned int  m_id;
    enum genre    m_genre;
    float         m_score;
    std::string   m_composerName, m_title, m_date, m_path;
};

#endif  // SRC_SONGITEM_HPP_
