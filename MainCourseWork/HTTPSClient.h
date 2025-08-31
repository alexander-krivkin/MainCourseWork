#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>

#include <iostream>
#include <string>

#include "shared.h"


namespace beast = boost::beast;     // <boost/beast.hpp>
namespace http = beast::http;       // <boost/beast/http.hpp>
namespace net = boost::asio;        // <boost/asio.hpp>
namespace ssl = net::ssl;           // <boost/asio/ssl.hpp>
namespace certify = boost::certify; // <boost/certify.hpp>
using tcp = net::ip::tcp;           // <boost/asio/ip/tcp.hpp>


namespace ak
{
    class HTTPSClient final
    {
    public:
        HTTPSClient() {}
        explicit HTTPSClient(const HTTPSClient& obj) = delete;
        ~HTTPSClient() {}
        HTTPSClient& operator=(const HTTPSClient& obj) = delete;

        std::string downloadHostData(const Host host);
    };
}