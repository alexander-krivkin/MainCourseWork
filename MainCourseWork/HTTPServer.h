#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/config.hpp>

#include <iostream>
#include <string>
#include <set>
#include <map>

#include "shared.h"
#include "PostgresDBClient.h"


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


namespace ak
{
    class HTTPServer final
    {
    public:
        HTTPServer() = delete;
        explicit HTTPServer(const GeneralState& state);
        explicit HTTPServer(const HTTPServer& obj) = delete;
        ~HTTPServer();
        HTTPServer& operator=(const HTTPServer& obj) = delete;

        void stop();

    private:
        void run_();
        void runSession_(tcp::socket& socket);
        http::message_generator handleRequest_(http::request<http::string_body>& request);
        std::multimap<uint32_t, SearchResult> getPostgresDbData_(const std::set<std::string>&searchWords);

        std::unique_ptr<std::jthread> upThread_{};

        std::atomic<bool> stopped_;
        GeneralState state_{};
    };
}