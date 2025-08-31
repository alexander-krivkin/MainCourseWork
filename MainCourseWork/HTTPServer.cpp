#include "HTTPServer.h"


namespace ak
{
	HTTPServer::HTTPServer(const Server server)
	{
		auto const address = net::ip::make_address(server.httpServerAddress);
		auto const port = static_cast<unsigned short>(server.httpServerPort);
		auto const doc_root = std::make_shared<std::string>(".");

		net::io_context ioc{ 1 };
		tcp::acceptor acceptor{ ioc, {address, port} };

		std::stringstream strS{};
		strS << "HTTPServer::HTTPServer: HTTP сервер запущен, ожидание соединения от клиента, адрес "
			<< server.httpServerAddress << ":" << server.httpServerPort;
		postLogMessage(strS.str());

		while (true)
		{
			tcp::socket socket{ ioc };
			acceptor.accept(socket);
			runSession_(socket, doc_root);
		}
	}

	void HTTPServer::runSession_(tcp::socket& socket,
		std::shared_ptr<std::string const> const& doc_root)
	{
		beast::error_code ec;
		beast::flat_buffer buffer{};

		while (true)
		{
			http::request<http::string_body> request;
			http::read(socket, buffer, request, ec);
			if (ec == http::error::end_of_stream) { break; }
			if (ec)
			{
				throw boost::system::system_error{ ec };
			}

			http::message_generator msg =
				handle_request(*doc_root, std::move(request));

			bool keep_alive = msg.keep_alive();
			beast::write(socket, std::move(msg), ec);
			if (ec)
			{
				throw boost::system::system_error{ ec };
			}
			if (!keep_alive) { break; }
		}

		socket.shutdown(tcp::socket::shutdown_send, ec);
	}
}