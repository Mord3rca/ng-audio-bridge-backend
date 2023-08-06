#ifndef SRC_FILTER_HPP_
#define SRC_FILTER_HPP_

extern "C" {
    #include <sqlite3.h>
}

#include <pistache/router.h>

#include <regex>
#include <string>
#include <vector>

#include "songItem.hpp"

class IFilter {
 public:
    IFilter() {}
    ~IFilter() {}

    virtual void set(const Pistache::Rest::Request&) = 0;
    virtual bool validate() const noexcept = 0;

    virtual const std::string getQuery() const noexcept = 0;
};

#ifdef NG_AUDIO_BRIDGE_COMPAT
class AudioBridgeFilter : public IFilter {
 public:
    AudioBridgeFilter();
    ~AudioBridgeFilter();

    void set(const Pistache::Rest::Request&);
    bool validate() const noexcept;

    const std::string getQuery() const noexcept;

 private:
    float m_minscore, m_maxscore;
    std::string m_mindate, m_maxdate;

    std::vector<enum genre> m_allowedgenre;
};
#endif  // NG_AUDIO_BRIDGE_COMPAT

class APIFilter : public IFilter {
 public:
    APIFilter();
    ~APIFilter();

    void set(const Pistache::Rest::Request&);
    bool validate() const noexcept;

    const std::string getQuery() const noexcept;

 private:
    std::string m_mindate, m_maxdate;
    float m_minscore, m_maxscore;
    bool m_allowUnrated;

    std::vector<enum genre> m_allowedgenre;
};

class APIFilterComposer : public IFilter {
 public:
    APIFilterComposer();
    ~APIFilterComposer();

    void set(const Pistache::Rest::Request&);
    bool validate() const noexcept;

    const std::string getQuery() const noexcept;

 private:
    std::string m_composer;
};

class APIFilterRandom : public IFilter {
 public:
    APIFilterRandom() {}
    ~APIFilterRandom() {}

    void set(const Pistache::Rest::Request&) {}
    bool validate() const noexcept { return true; }

    const std::string getQuery() const noexcept
    { return "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks ORDER BY RANDOM() LIMIT 1;"; }
};

class APIFilterRange : public IFilter {
 public:
    APIFilterRange() : m_min(0), m_max(0) {}
    ~APIFilterRange() {}

    void set(const Pistache::Rest::Request&);
    bool validate() const noexcept
    { return m_min != 0 && m_max != 0;}

    const std::string getQuery() const noexcept
    { return  "SELECT id,title,composer,score,genre,submission_date,url FROM Tracks WHERE id BETWEEN "
              + std::to_string(m_min) + " AND " + std::to_string(m_max) +";"; }

 private:
    unsigned int m_min, m_max;
};

static const std::regex regdate("^\\d{4}/\\d{2}/\\d{2}$");

#endif  // SRC_FILTER_HPP_
