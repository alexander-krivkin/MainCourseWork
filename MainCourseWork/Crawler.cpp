#include "Crawler.h"


namespace ak
{
	Crawler::Crawler(const GeneralState& state) : state_(state)
	{
		hostCount_.store(0);
		stopped_.store(false);

		runThreadPull_();
		recreatePostgresDbTables_();

		std::stringstream strS{};
		strS << "Crawler::Crawler: краулер запущен, глубина поиска " << state_.searchDepth << ", лимит поиска " << state_.searchLimit;
		postLogMessage(strS.str());

		Host indexedHost = { 0, state_.httpsHost, state_.httpsHostParams, state_.httpsPort };
		upThreadPull_->submit(std::bind(&Crawler::parseHost, this, indexedHost));
		//parseHost(indexedHost);
	}

	Crawler::~Crawler()
	{
		if (!stopped_.load()) stop();
	}

	void Crawler::parseHost(const Host host)
	{
		if ((host.level >= state_.searchDepth)
			|| (hostCount_.load() >= state_.searchLimit))
		{
			stopped_.store(true);
			return;
		}

		auto downloadedHostData{ downloadHostData_(host) };
		auto indexedHostData{ indexHostData_(host, downloadedHostData) };
		insertPostgresDbIndexedHostData_(indexedHostData);
		hostCount_++;

		std::stringstream strS{};
		strS << "Crawler::parseHost: уровень " << (host.level + 1) << ", хост " << hostCount_.load()
			<< " отработан, адрес " << indexedHostData.httpsHostWithParams;
		postLogMessage(strS.str());

		for (const auto& indexedHost : indexedHostData.indexedHosts)
		{
			upThreadPull_->submit(std::bind(&Crawler::parseHost, this, indexedHost));
			//parseHost(indexedHost);
		}
	}

	void Crawler::waitAndStop()
	{
		while (true)
		{
			if (stopped_.load()) { break; }
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(workTimeout));
		stop();
	}

	void Crawler::stop()
	{
		stopThreadPull_();
		stopped_.store(true);

		postLogMessage("Crawler::stop: краулер остановлен");
	}

	void Crawler::runThreadPull_()
	{
		// ещё один поток нужен для сервера
		uint32_t threadsCount = state_.maxThreads < (std::thread::hardware_concurrency() - 1) ?
			state_.maxThreads : (std::thread::hardware_concurrency() - 1);
		upThreadPull_ = std::unique_ptr<ThreadPull>(new ThreadPull{ threadsCount });
	}

	void Crawler::stopThreadPull_()
	{
		upThreadPull_->stop();
	}

	void Crawler::recreatePostgresDbTables_()
	{
		std::unique_ptr<PostgresDBClient> upPostgresDBClient(new PostgresDBClient{
			{ state_.dbHost, state_.dbPort, state_.dbName, state_.dbUser, state_.dbPassword } });

		upPostgresDBClient->deleteTables();
		upPostgresDBClient->createTables();
	}

	std::string Crawler::downloadHostData_(const Host host)
	{
		std::unique_ptr<HTTPSClient> upHTTPSClient(new HTTPSClient{});
		return upHTTPSClient->downloadHostData(host);
	}

	IndexedHostData Crawler::indexHostData_(const Host host, const std::string& downloadedHostData)
	{
		IndexedHostData ret{};
		ret.level = host.level;
		ret.httpsHostWithParams = host.httpsHost + host.httpsHostParams;

		std::string str{ downloadedHostData }, tempStr{}, ref{};
		size_t start{}, middle{}, end{};

		// 1. Преобразование кодировки из UTF8 в CP1251
		start = str.find("charset=UTF-8", 0);
		if (start != std::string::npos)
		{
			str = utf8ToCp1251(str);
		}

		// 2. Выгрузка заголовка
		start = str.find("<title>", 0);
		end = str.find("</title>", start);
		ret.indexedTitle = str.substr(start + 7, end - (start + 7));
		if (ret.indexedTitle.size() > 210)
		{
			ret.indexedTitle = ret.indexedTitle.substr(0, 210);
		}

		// 3. Выгрузка абсолютных ссылок HTTPS
		tempStr = str;
		while (true)
		{
			start = tempStr.find("<a href=\"https://", 0);
			if (start == std::string::npos) { break; }

			tempStr = tempStr.substr(start + 17, tempStr.size() - 1);
			middle = tempStr.find("\"", 0);
			end = tempStr.find("</a>", 0);

			ref = tempStr.substr(0, middle);
			tempStr = tempStr.substr(end + 4, tempStr.size() - 1);

			middle = ref.find("#", 0);
			if (middle != std::string::npos)
			{
				ref = ref.substr(0, middle);
			}

			middle = ref.find("/", 0);
			if (middle == std::string::npos)
			{
				ret.indexedHosts.insert(
					{
							host.level + 1,
								ref.substr(0, ref.size() - 1),
								"/",
								host.httpsPort
					});
			}
			else
			{
				ret.indexedHosts.insert(
					{
							host.level + 1,
								ref.substr(0, middle),
								ref.substr(middle, ref.size() - 1),
								host.httpsPort
					});
			}
		}

		// 4. Выгрузка относительных ссылок HTTPS
		tempStr = str;
		while (true)
		{
			start = tempStr.find("<a href=\"/", 0);
			if (start == std::string::npos) { break; }

			tempStr = tempStr.substr(start + 10, tempStr.size() - 1);
			middle = tempStr.find("\"", 0);
			end = tempStr.find("</a>", 0);

			ref = tempStr.substr(0, middle);
			tempStr = tempStr.substr(end + 4, tempStr.size() - 1);

			middle = ref.find("#", 0);
			if (middle != std::string::npos)
			{
				ref = ref.substr(0, middle);
			}

			if (ref[0] != '/') { ref = "/" + ref; }
			ret.indexedHosts.insert(
				{
						host.level + 1,
							host.httpsHost,
							ref,
							host.httpsPort
				});
		}

		// 5. Отсечение до тела страницы
		start = str.find("<body>", 0);
		end = str.find("</body>", start);
		str = str.substr(start + 6, end - (start + 6));

		// 6. Удаление скриптов
		while (true)
		{
			start = str.find("<script", 0);
			if (start == std::string::npos) { break; }

			end = str.find("</script>", start);
			if (end == std::string::npos) { break; }

			str.erase(start, end + 9);
		}

		// 7. Приведение к нижнему регистру, удаление тэгов и очистка до букв и пробелов
		str = toLower(str);
		str = eraseTags(str);
		str = toLetters(str);

		size_t strSize{};
		do
		{
			strSize = str.size();
			str = replaceRegex(str, "  ", " ");
		} while (strSize != str.size());

		// 8. Выгрузка карты слов длиной от 3 до 32 символов включительно
		std::stringstream strS{ str };
		std::vector<std::string> indexedWords{};
		std::string word{};
		while (strS >> word)
		{
			if ((word.size() >= 3) && (word.size() <= 32))
			{
				indexedWords.push_back(word);
			}
		}
		ret.indexedWords = toCounterMap(indexedWords);

		return ret;
	}

	void Crawler::insertPostgresDbIndexedHostData_(const IndexedHostData& indexedHostData)
	{
		std::unique_ptr<PostgresDBClient> upPostgresDBClient(new PostgresDBClient{
			{ state_.dbHost, state_.dbPort, state_.dbName, state_.dbUser, state_.dbPassword } });

		upPostgresDBClient->addHostAndWords(indexedHostData.httpsHostWithParams,
			indexedHostData.indexedTitle, indexedHostData.indexedWords);
	}
}