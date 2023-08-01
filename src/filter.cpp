#include "filter.hpp"

#include <json/json.h>

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

Json::Value readJSONFromRequest(const Pistache::Rest::Request &req) {
    Json::CharReaderBuilder builder; Json::Value root;
    Json::CharReader *j = builder.newCharReader();
    const char *body = req.body().c_str();
    std::string err;

    j->parse(body, body + req.body().length(), &root, &err);
    delete j;
    return root;
}

AudioBridgeFilter::AudioBridgeFilter() :
    m_minscore(0), m_maxscore(5),
    m_mindate("2003/01/01"),
    m_allowedgenre(default_allowed_genre)
{}

AudioBridgeFilter::~AudioBridgeFilter() {}

void AudioBridgeFilter::set(const Pistache::Rest::Request &req) {
    Json::CharReaderBuilder builder; Json::Value root;
    auto arg = req.query().get("filterJSON");
    if (!arg) return;

    auto jsonstr = arg.value();
    Json::CharReader *json_read = builder.newCharReader();
    std::string err;

    if (!json_read->parse(jsonstr.c_str(), jsonstr.c_str() + jsonstr.length(), &root, &err))
        goto end;

    m_minscore = root.get("minScore", 0).asFloat();
    m_maxscore = root.get("maxScore", 5).asFloat();

    std::string::size_type pos;
    m_mindate = root.get("minDate", "2003/01/01").asString();
    while ((pos = m_mindate.find('-')) != std::string::npos)
        m_mindate.replace(pos, 1, "/");

    m_maxdate = root.get("maxDate", "").asString();
    while ((pos = m_maxdate.find('-')) != std::string::npos)
        m_maxdate.replace(pos, 1, "/");

    if (root["genres"].isArray()) {
        m_allowedgenre.clear();
        for (auto i : root["genres"])
            m_allowedgenre.push_back(strToGenre(i.asString()));
    }

end:
    delete json_read;
}

bool AudioBridgeFilter::validate() const noexcept {
    if (!std::regex_match(m_mindate, regdate)) return false;

    if (!m_maxdate.empty())
        if (!std::regex_match(m_maxdate, regdate)) return false;

    return true;
}

const std::string AudioBridgeFilter::getQuery() const noexcept {
    std::ostringstream sql_query; std::vector<std::string> conditions;
    sql_query << "SELECT id,title,composer,score,genre,submission_date,url "
              << "FROM Tracks WHERE id IN (SELECT id FROM Tracks";

    if (!m_maxdate.empty() && m_mindate != "2003/01/01")
        conditions.push_back("(submission_date BETWEEN \"" + m_mindate + "\" AND \"" + m_maxdate +"\")");

    if (m_minscore != 0 || m_maxscore != 5)
        conditions.push_back("(score BETWEEN " + std::to_string(m_minscore) +
                             " AND " + std::to_string(m_maxscore) + ")");

    if (m_allowedgenre.size() < 48) {
        if (m_allowedgenre.size() == 1) {
            conditions.push_back("genre=" + std::to_string(static_cast<int>(m_allowedgenre[0])));
        } else {
            std::string tmp = "genre IN (";
            for (size_t i = 0; i < m_allowedgenre.size(); i++) {
                tmp += std::to_string(static_cast<int>(m_allowedgenre[i]));
                if (i != m_allowedgenre.size() - 1)
                    tmp += ", ";
            }
            tmp += ')';
            conditions.push_back(tmp);
        }
    }

    if (!conditions.empty()) sql_query << " WHERE ";
    for (std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); i++) {
        sql_query << *i;
        if (i != conditions.end()) sql_query << " AND ";
    }

    sql_query << " ORDER BY RANDOM() LIMIT 25);";

    return sql_query.str();
}

APIFilter::APIFilter() :
    m_mindate("2003/01/01"), m_maxdate(""),
    m_minscore(0), m_maxscore(5),
    m_allowUnrated(true) {
    m_allowedgenre = default_allowed_genre;
}

APIFilter::~APIFilter() {}

void APIFilter::set(const Pistache::Rest::Request &req) {
    Json::Value root = readJSONFromRequest(req);

    m_mindate = root.get("minDate", "2003/01/01").asString();
    m_maxdate = root.get("maxDate", "").asString();
    m_minscore = root.get("minScore", 0).asFloat();
    m_maxscore = root.get("maxScore", 5).asFloat();
    m_allowUnrated = root.get("allowUnrated", true).asBool();

    if (!root["genres"].isArray()) return;

    m_allowedgenre.clear();
    for (auto i : root["genres"]) {
        if (!i.isInt()) continue;

        try {
            m_allowedgenre.push_back(static_cast<enum genre>(i.asInt()));
        } catch (std::exception&) {continue;}
    }
}

bool APIFilter::validate() const noexcept {
    // Date check.
    if (!std::regex_match(m_mindate, regdate)) return false;

    if (!m_maxdate.empty())
        if (!std::regex_match(m_maxdate, regdate)) return false;

    if (m_minscore > m_maxscore) return false;

    if (!(m_minscore >= 0 && m_minscore <= 5) || !( m_maxscore >= 0 && m_maxscore <= 5))
        return false;

    return true;
}

const std::string APIFilter::getQuery() const noexcept {
    std::ostringstream sql_query; std::vector<std::string> conditions;
    sql_query << "SELECT id,title,composer,score,genre,submission_date,url "
              << "FROM Tracks WHERE id IN (SELECT id FROM Tracks";

    if (m_minscore != 0 || m_maxscore != 5) {
        std::string tmp = "(score BETWEEN " + std::to_string(m_minscore) +
                          " AND " + std::to_string(m_maxscore) + ")";

        if (m_allowUnrated)
            tmp = "( " + tmp + "OR score=-1)";

        conditions.push_back(tmp);
    }

    if (m_mindate != "2003/01/01" || !m_maxdate.empty())
        conditions.push_back("(submission_date BETWEEN \"" + m_mindate + "\" AND \"" + m_maxdate + "\")");

    if (m_allowedgenre.size() < 48) {
        std::string tmp = "genre IN (";
        for (size_t i = 0; i < m_allowedgenre.size(); i++) {
            tmp += std::to_string(static_cast<int>(m_allowedgenre[i]));
            if (i != m_allowedgenre.size() - 1)
                tmp += ", ";
        }
        tmp += ')';
        conditions.push_back(tmp);
    }

    if (!conditions.empty()) sql_query << " WHERE ";
    for (std::vector<std::string>::const_iterator i = conditions.begin(); i != conditions.end(); i++) {
        sql_query << *i;
        if (i != conditions.end()) sql_query << " AND ";
    }
    sql_query << " ORDER BY RANDOM() LIMIT 25);";

    return sql_query.str();
}

APIFilterComposer::APIFilterComposer() {}

APIFilterComposer::~APIFilterComposer() {}

void APIFilterComposer::set(const Pistache::Rest::Request &req) {
    m_composer = readJSONFromRequest(req).get("composer", "").asString();
}

bool APIFilterComposer::validate() const noexcept {
    return !m_composer.empty();
}

const std::string APIFilterComposer::getQuery() const noexcept {
    std::string result; char *buff;
    buff = sqlite3_mprintf("SELECT id,title,composer,score,genre,submission_date,url "
                           "FROM Tracks WHERE composer LIKE '%q';", m_composer.c_str());
    result = buff;
    sqlite3_free(buff);
    return result;
}

void APIFilterRange::set(const Pistache::Rest::Request &req) {
    auto root = readJSONFromRequest(req);
    auto mul  = root.get("mul", 1).asInt();
    auto page = root.get("page", 1).asInt();

    m_min = page*mul + 1;
    m_max = (page+1)*mul;
}
