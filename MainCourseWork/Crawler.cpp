#include "Crawler.h"


namespace ak
{
	Crawler::Crawler(const GeneralState& state) : state_(state)
	{
		stopped_ = false;
		runThreadPull_();
		recreatePostgresDbTables_();

		std::stringstream strS{};
		strS << "Crawler::Crawler: ������� �������, ������� ������ " << state_.searchDepth << ", ����� ������ " << state_.searchLimit;
		postLogMessage(strS.str());

		Host indexedHost = { 0, state_.httpsHost, state_.httpsHostParams, state_.httpsPort };
		upThreadPull_->submit(std::bind(&Crawler::parseHost, this, indexedHost));
		//parseHost(indexedHost);
	}

	Crawler::~Crawler()
	{
		if (!stopped_) stop();
	}

	void Crawler::parseHost(const Host host)
	{
		if (stopped_) return;

		auto downloadedHostData{ downloadHostData_(host) };
		auto indexedHostData{ indexHostData_(host, std::cref(downloadedHostData)) };
		insertPostgresDbIndexedHostData_(std::cref(indexedHostData));

		if (host.level < state_.searchDepth)
		{
			for (const auto& indexedHost : indexedHostData.indexedHosts)
			{
				upThreadPull_->submit(std::bind(&Crawler::parseHost, this, indexedHost));
				//parseHost(indexedHost);
			}
		}

		std::stringstream strS{};
		strS << "Crawler::parseHost: ���� " << ++hostCount_ << " ���������, ����� " << indexedHostData.httpsHostWithParams;
		postLogMessage(strS.str());

		if (host.level == (state_.searchDepth - 1)
			|| (hostCount_ >= state_.searchLimit)
			|| (!indexedHostData.indexedHosts.size()))
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(workTimeout));
			stopped_ = true;
		}
	}

	void Crawler::waitAndStop()
	{
		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(workTimeout));

			if (stopped_) { break; }
		}
		stop();
	}

	void Crawler::stop()
	{
		stopThreadPull_();
		stopped_ = true;

		postLogMessage("Crawler::stop: ������� ����������");
	}

	void Crawler::runThreadPull_()
	{
		uint32_t threadsCount = state_.maxThreads < std::thread::hardware_concurrency() ?
			state_.maxThreads : std::thread::hardware_concurrency();
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
		ret.level = host.level + 1;
		ret.httpsHostWithParams = host.httpsHost + host.httpsHostParams;

		std::string str{ downloadedHostData }, tempStr{}, ref{};
		size_t start{}, middle{}, end{};

		// 1. �������������� ��������� �� UTF8 � CP1251
		start = str.find("charset=UTF-8", 0);
		if (start != std::string::npos)
		{
			str = utf8ToCp1251(str);
		}

		// 2. �������� ���������
		start = str.find("<title>", 0);
		end = str.find("</title>", start);
		ret.indexedTitle = str.substr(start + 7, end - (start + 7));
		if (ret.indexedTitle.size() > 210)
		{
			ret.indexedTitle = ret.indexedTitle.substr(0, 210);
		}

		// 3. ���������� � ������� ��������
		str = toLower(str);

		// 4. �������� ���������� ������ HTTPS
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
			middle = ref.find("/", 0);

			ret.indexedHosts.insert(
				{
						host.level + 1,
							ref.substr(0, middle),
							ref.substr(middle, ref.size() - 1),
							host.httpsPort
				});
		}

		// 5. �������� ������������� ������ HTTPS
		tempStr = str;
		while (true)
		{
			start = tempStr.find("<a href=\"/", 0);
			if (start == std::string::npos) { break; }

			tempStr = tempStr.substr(start + 10, tempStr.size() - 1);
			middle = tempStr.find("\"", 0);
			end = tempStr.find("</a>", 0);

			ref = tempStr.substr(0, middle);
			if (ref[0] != '/') { ref = "/" + ref; }
			tempStr = tempStr.substr(end + 4, tempStr.size() - 1);

			ret.indexedHosts.insert(
				{
						host.level + 1,
							host.httpsHost,
							ref,
							host.httpsPort
				});
		}

		// 6. ��������� �� ���� ��������
		start = str.find("<body>", 0);
		end = str.find("</body>", start);
		str = str.substr(start + 6, end - (start + 6));

		// 7. ������� ������ �� ������� ���� � ��������
		str = toCyrillicWords(str);
		int strSize{};
		do
		{
			strSize = str.size();
			str = findAndReplaceRegex(str, "  ", " ");
		} while (strSize != str.size());

		// 8. �������� ����� ���� ������ �� 3 �� 32 �������� ������������
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