#ifndef SRC_AUDIOSERVER_HPP_
#define SRC_AUDIOSERVER_HPP_

#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>

#include <memory>

#include "AudioDatabase.hpp"

class AudioServer {
 public:
    explicit AudioServer(Pistache::Address);

    AudioDatabase* getDatabase();

    void init(size_t = 2);
    void start();

 private:
    void setupRoutes();

    void getVersion(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);

    void getCrossdomain(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getAudioBridgeMp3(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getAudioBridgeFilter(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);

    void getGenres(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getTrackById(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getRandomTrack(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getFilter(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void getFilterComposer(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);

    Pistache::Rest::Router router;
    std::shared_ptr<Pistache::Http::Endpoint> httpEndpoint;
    std::shared_ptr<AudioDatabase> m_db;
};

#endif  // SRC_AUDIOSERVER_HPP_
