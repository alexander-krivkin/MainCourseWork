#include "HTTPServer.h"


namespace ak
{
	HTTPServer::HTTPServer(const GeneralState& state) : state_(state)
	{
		upThread_ = std::unique_ptr<std::jthread>(
			new std::jthread(std::bind(&HTTPServer::run_, this)));
	}

	HTTPServer::~HTTPServer()
	{
		if (!stopped_.load()) { stop(); }
	}

	void HTTPServer::run_()
	{
		std::stringstream strS{};
		strS << "HTTPServer::HTTPServer: HTTP сервер запущен, адрес "
			<< state_.httpServerAddress << ":" << state_.httpServerPort;
		postLogMessage(strS.str());

		stopped_.store(false);
		auto const address = net::ip::make_address(state_.httpServerAddress);
		auto const port = static_cast<unsigned short>(state_.httpServerPort);

		net::io_context ioc{ 1 };
		tcp::acceptor acceptor{ ioc, {address, port} };

		while (true)
		{
			if (stopped_.load()) { break; }

			tcp::socket socket{ ioc };
			acceptor.accept(socket);
			runSession_(socket);
		}
	}

	void HTTPServer::stop()
	{
		stopped_.store(true);
		upThread_->detach();

		postLogMessage("HTTPServer::stop: HTTP сервер остановлен");
	}

	void HTTPServer::runSession_(tcp::socket& socket)
	{
		if (stopped_.load()) { return; }

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

			if (stopped_.load()) { break; }
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
			postLogMessage("HTTPServer::handleRequest_: отработка метода GET");

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
			postLogMessage("HTTPServer::handleRequest_: отработка метода POST");

			http::response<http::string_body> response{
				 http::status::ok, request.version() };

			response.set(http::field::server, BOOST_BEAST_VERSION_STRING);
			response.set(http::field::content_type, "text/html");
			response.keep_alive(request.keep_alive());

			// 1. Приведение к нижнему регистру, удаление тэгов и очистка до букв и пробелов
			std::string str{ request.body() };
			str = urlDecode(str);
			str = toLower(str);
			str.erase(0, 8); // удаление "request="
			str = eraseTags(str);
			str = toLetters(str);

			size_t strSize{};
			do
			{
				strSize = str.size();
				str = replaceRegex(str, "  ", " ");
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

			if (responseStr.str().size() == 16)
			{
				responseStr << "<li>Результаты поиска отсутствуют</li><br>";
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

	std::multimap<uint32_t, SearchResult> HTTPServer::getPostgresDbData_(
		const std::set<std::string>& searchWords)
	{
		std::unique_ptr<PostgresDBClient> upPostgresDBClient(new PostgresDBClient{
			{ state_.dbHost, state_.dbPort, state_.dbName, state_.dbUser, state_.dbPassword } });
		
		return upPostgresDBClient->getSearchResults(searchWords);
	}
}