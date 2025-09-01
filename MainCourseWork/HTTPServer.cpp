#include "HTTPServer.h"


namespace ak
{
	HTTPServer::HTTPServer(const GeneralState& state) : state_(state)
	{
		auto const address = net::ip::make_address(state_.httpServerAddress);
		auto const port = static_cast<unsigned short>(state_.httpServerPort);

		net::io_context ioc{ 1 };
		tcp::acceptor acceptor{ ioc, {address, port} };

		std::stringstream strS{};
		strS << "---------------------------------------------------------------------------";
		postLogMessage(strS.str());
		strS = {};
		strS << "HTTPServer::HTTPServer: HTTP сервер запущен, ожидание соединения от клиента, адрес "
			<< state_.httpServerAddress << ":" << state_.httpServerPort;
		postLogMessage(strS.str());

		while (true)
		{
			tcp::socket socket{ ioc };
			acceptor.accept(socket);
			runSession_(socket);
		}
	}

	void HTTPServer::runSession_(tcp::socket& socket)
	{
		beast::error_code ec;
		beast::flat_buffer buffer{};

		postLogMessage("HTTPServer::runSession_: сессия открыта");

		while (true)
		{
			http::request<http::string_body> request;
			http::read(socket, buffer, request, ec);
			if (ec == http::error::end_of_stream) { break; }
			if (ec)
			{
				throw boost::system::system_error{ ec };
			}

			auto msg = handleRequest_(request);

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

	http::message_generator HTTPServer::handleRequest_(
		http::request<http::string_body>& request)
	{
		if ((request.method() != http::verb::get) &&
			(request.method() != http::verb::post))
		{
			throw(std::exception("HTTPServer::handleRequest_: необрабатываемый HTTP-метод"));
		}

		// GET
		if ((request.method() == http::verb::get))
		{
			http::response<http::string_body> response{
				http::status::ok, request.version() };

			response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			response.set(http::field::content_type, "text/html");
			response.keep_alive(request.keep_alive());

			response.body() = "<html><head>"
				"<meta http-equiv = \"Content-Type\" content = \"text/html; charset=windows-1251\"/>"
				"<title>Дипломная работа</title></head>"
				"<body><form action = \"/submit\" method = \"post\">"
				"<label for = \"request\">Введите поисковый запрос: </label>"
				"<input type = \"text\" id = \"request\" name = \"request\" required>"
				"<button type = \"submit\">Поиск</button>"
				"</form></body></html>";

			response.prepare_payload();
			return response;
		}

		// POST
		if ((request.method() == http::verb::post)
			&& (request.target() == "/submit"))
		{
			http::response<http::string_body> response{
				 http::status::ok, request.version() };

			response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			response.set(http::field::content_type, "text/html");
			response.keep_alive(request.keep_alive());

			// 1. Приведение к нижнему регистру и очистка текста до русских букв и пробелов
			std::string str{ request.body() };
			str = urlDecode(str);
			str = toLower(str);
			str = toCyrillicWords(str);

			size_t strSize{};
			do
			{
				strSize = str.size();
				str = findAndReplaceRegex(str, "  ", " ");
			} while (strSize != str.size());

			// 2. Выгрузка карты слов длиной от 3 до 32 символов включительно
			std::stringstream strS{ str };
			std::set<std::string> searchWords{};
			std::string word{};
			while (strS >> word)
			{
				if ((word.size() >= 3) && (word.size() <= 32))
				{
					searchWords.insert(word);
				}
			}

			// 3. Выгрузка результатов поиска из базы данных
			auto searchResults = getPostgresDbData_(searchWords);

			// 4. Вывод результатов поиска
			std::stringstream responseStr{};

			responseStr << "<html><body><ul>";

			for (auto it = searchResults.rbegin(); it != searchResults.rend(); it++)
			{
				responseStr << "<li>Рейтинг: " << (*it).first
					<< "<br>" << (*it).second.hostTitle
					<< "<br><a href =\"" << (*it).second.host
					<< "\">" << (*it).second.host
					<< "</a></li><br>";
			}
			responseStr << "</ul></body></html>";

			response.body() = responseStr.str();
			response.prepare_payload();
			return response;
		}
		else if ((request.method() == http::verb::post)
			&& (request.target() != "/submit"))
		{
			throw(std::exception("HTTPServer::handleRequest_: внутренняя ошибка"));
		}
	}

	std::map<uint32_t, SearchResult> HTTPServer::getPostgresDbData_(
		const std::set<std::string>& searchWords)
	{
		std::unique_ptr<PostgresDBClient> upPostgresDBClient(new PostgresDBClient{
			{ state_.dbHost, state_.dbPort, state_.dbName, state_.dbUser, state_.dbPassword } });
		
		return upPostgresDBClient->getSearchResults(searchWords);
	}
}