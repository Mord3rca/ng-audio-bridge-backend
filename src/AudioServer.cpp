#include "AudioServer.hpp"

#include <string>
#include <sstream>

using namespace Pistache;

static const std::string _crossdomain(
"<?xml version=\"1.0\"?>\n"
"<!DOCTYPE cross-domain-policy SYSTEM \"http://www.adobe.com/xml/dtds/cross-domain-policy.dtd\">\n"
"<cross-domain-policy>\n"
"    <site-control permitted-cross-domain-policies=\"master-only\"/>\n"
"    <allow-access-from domain=\"*\" secure=\"false\"/>\n"
"    <allow-http-request-headers-from domain=\"*\" headers=\"*\"/>\n"
"</cross-domain-policy>\n"
);

AudioServer::AudioServer(Address addr) :
    httpEndpoint(std::make_shared<Http::Endpoint>(addr)),
    m_db(std::make_shared<AudioDatabase>())
{}

void AudioServer::init(size_t th)
{
    auto opts = Http::Endpoint::options()
                    .threads(static_cast<int>(th))
                    .flags(Tcp::Options::ReuseAddr);
    httpEndpoint->init(opts);
    setupRoutes();
}

void AudioServer::start()
{
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

AudioDatabase* AudioServer::getDatabase()
{
    return m_db.get();
}

void AudioServer::setupRoutes()
{
    using namespace Rest;

    // Old API Routes
    Routes::Get(router, "/crossdomain.xml", Routes::bind(&AudioServer::getCrossdomain, this));
    Routes::Get(router, "/audio/serve/:unused/:id", Routes::bind(&AudioServer::getAudioBridgeMp3, this));
    
    Routes::Post(router, "/api/filter/old", Routes::bind(&AudioServer::getAudioBridgeMp3, this));
    Routes::Post(router, "/Radio2/FilterBridge.php", Routes::bind(&AudioServer::postAudioBridgeFilter, this));
    
    // New API Routes
    Routes::Get(router, "/api/genres", Routes::bind(&AudioServer::getGenres, this));
    Routes::Get(router, "/api/version", Routes::bind(&AudioServer::getVersion, this));
    Routes::Get(router, "/api/track/:id", Routes::bind(&AudioServer::getTrackById, this));
    Routes::Get(router, "/api/track/random", Routes::bind(&AudioServer::getRandomTrack, this));
    
    Routes::Post(router, "/api/filter", Routes::bind(&AudioServer::postFilter, this));
    Routes::Post(router, "/api/filter/composer", Routes::bind(&AudioServer::postFilterComposer, this));
}

void AudioServer::getVersion(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    response.send(Http::Code::Ok, "0.1", MIME(Text, Plain));
}

void AudioServer::getCrossdomain(const Rest::Request& request, Http::ResponseWriter response)
{
    response.send(Http::Code::Ok, _crossdomain, MIME(Text, Xml));
}

void AudioServer::postAudioBridgeFilter(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    AudioBridgeFilter filter; filter.set(req);

    if(!filter.validate())
    {
        response.send(Http::Code::Internal_Server_Error, "Cannot validate filter");
        return;
    }

    response.send(Http::Code::Ok, "{\"ResultSet\":" + m_db->getViaFilter(filter).toJson() + "}");
}

void AudioServer::getAudioBridgeMp3(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    auto id = req.param(":id").as<std::string>(); // Process required to get rid of '.mp3'
    auto rslt = m_db->getSongByID(std::atoi(id.substr(0, id.length()-4).c_str()));
    if(rslt.isEmpty())
    {
        response.send(Http::Code::Not_Found, "Song ID not found in database");
        return;
    }
    
    response.headers().add<Http::Header::Location>(rslt[0].getURL());
    response.send(Http::Code::Moved_Permanently);
}

void AudioServer::getTrackById(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    auto rslt = m_db->getSongByID(req.param(":id").as<int>());
    if(rslt.isEmpty())
    {
        response.send(Http::Code::Not_Found, "Song ID not found in database");
        return;
    }
    
    response.headers().add<Http::Header::Location>(rslt[0].getURL());
    response.send(Http::Code::Moved_Permanently);
}

void AudioServer::getRandomTrack(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    auto rslt = m_db->getViaFilter( APIFilterRandom() );
    
    if(rslt.isEmpty())
    {
        response.send(Http::Code::Internal_Server_Error, "Randomness was too random");
        return;
    }
    
    response.headers().add<Http::Header::Location>(rslt[0].getURL());
    response.send(Http::Code::Moved_Permanently);
}

void AudioServer::getGenres(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    std::stringstream stream;
    stream << "{\"genres\":[";
    for(auto j : m_db->getGenreList())
    {
        stream  << "{\"id\": " << std::to_string(std::get<0>(j))
                << ", \"name\": \"" << std::get<1>(j) << "\""
                << "}, ";
    }
    stream << "{}]}";
    
    response.send(Http::Code::Ok, stream.str(), MIME(Application, Json));
}

void AudioServer::postFilterComposer(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    APIFilterComposer filter; filter.set(req);
    
    if(!filter.validate())
    {
        response.send(Http::Code::Internal_Server_Error, "Cannot validate filter");
        return;
    }
    
    auto rslt = m_db->getViaFilter(filter);
    
    response.send(Http::Code::Ok, "{\"Tracks\":" + rslt.toJson() + "}", MIME(Application, Json));
}

void AudioServer::postFilter(const Pistache::Rest::Request &req, Pistache::Http::ResponseWriter response)
{
    
}