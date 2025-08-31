#include "HTTPSClient.h"


namespace ak
{
    std::string HTTPSClient::downloadHostData(const Host host)
    {
        net::io_context ioc{};
        ssl::context ctx{ ssl::context::sslv23_client };

        ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);
        ctx.set_default_verify_paths();
        certify::enable_native_https_server_verification(ctx);

        tcp::resolver resolver{ ioc };
        ssl::stream<tcp::socket> stream{ ioc, ctx };

        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.httpsHost.c_str()))
        {
            boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            throw boost::system::system_error{ ec };
        }

        auto const results = resolver.resolve(host.httpsHost, host.httpsPort);
        net::connect(stream.next_layer(), results.begin(), results.end());
        stream.handshake(ssl::stream_base::client);
        
        http::request<http::string_body> request{ http::verb::get, host.httpsHostParams, 11 };
        request.set(http::field::host, host.httpsHost);
        request.set(http::field::user_agent, userAgentName);

        http::write(stream, request);
        beast::flat_buffer buffer{};
        http::response<http::dynamic_body> response{};
        http::read(stream, buffer, response);

        std::stringstream ret{};
        ret << response;

        boost::system::error_code ec{};
        stream.lowest_layer().cancel(ec);
        stream.shutdown(ec);

        if (ec == ssl::error::stream_truncated)
        {
            ec.assign(0, ec.category());
        }
        else if (ec)
        {
            throw boost::system::system_error{ ec };
        }

        std::stringstream strS{};
        strS << "HTTPSClient::downloadHostData: данные с хоста "
            << host.httpsHost << host.httpsHostParams << " загружены, размер " << ret.str().length() << " байт";
        postLogMessage(strS.str());

        return ret.str();
    }
}