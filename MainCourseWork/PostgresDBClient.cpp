#include "PostgresDBClient.h"


namespace ak
{
	PostgresDBClient::PostgresDBClient(const PostgresDb& postgresDb) : postgresDb_(postgresDb)
	{
		std::stringstream strS1{};
		strS1 << "host=" << postgresDb_.dbHost
			<< " port = " << postgresDb_.dbPort
			<< " dbname = " << postgresDb_.dbName
			<< " user = " << postgresDb_.dbUser
			<< " password = " << postgresDb_.dbPassword;

		upConnection_ = std::make_unique<pqxx::connection>(strS1.str());

		std::stringstream strS{};
		strS << "PostgresDBClient::PostgresDBClient: установлено подключение к базе данных \"" << upConnection_->dbname() << "\"";
		postLogMessage(strS.str());
	}

	void PostgresDBClient::deleteTables()
	{
		std::string str1{
			"DROP TABLE IF EXISTS hosts_words CASCADE;" };
		std::string str2{
			"DROP TABLE IF EXISTS hosts CASCADE;" };
		std::string str3{
			"DROP TABLE IF EXISTS words CASCADE;" };

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		upWork_->exec(str1);
		upWork_->exec(str2);
		upWork_->exec(str3);
		upWork_->commit();

		std::stringstream strS{};
		strS << "PostgresDBClient::deleteTables: в базе данных \"" << upConnection_->dbname() << "\" таблицы удалены";
		postLogMessage(strS.str());
	}

	void PostgresDBClient::createTables()
	{
		std::string str1{
			"CREATE TABLE IF NOT EXISTS hosts( "
			"id SERIAL PRIMARY KEY, "
			"host VARCHAR(300) UNIQUE NOT NULL, "
			"host_title VARCHAR(210) "
			"); " };
		std::string str2{
			"CREATE TABLE IF NOT EXISTS words( "
			"id SERIAL PRIMARY KEY, "
			"word VARCHAR(33) UNIQUE NOT NULL "
			"); " };
		std::string str3{
			"CREATE TABLE IF NOT EXISTS hosts_words( "
			"host_id INTEGER NOT NULL REFERENCES hosts(id), "
			"word_id INTEGER NOT NULL REFERENCES words(id), "
			"word_frequency INTEGER NOT NULL, "
			"PRIMARY KEY (host_id, word_id) "
			"); " };

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		upWork_->exec(str1);
		upWork_->exec(str2);
		upWork_->exec(str3);
		upWork_->commit();

		std::stringstream strS{};
		strS << "PostgresDBClient::deleteTables: в базе данных \"" << upConnection_->dbname() << "\" таблицы созданы";
		postLogMessage(strS.str());
	}

	void PostgresDBClient::addHost(const std::string& host, const std::string& hostTitle)
	{
		std::string str1{
			"INSERT INTO hosts(host, host_title) "
			"VALUES($1, $2) "
			"ON CONFLICT DO NOTHING;" };

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		upWork_->exec(str1, pqxx::params{ host, hostTitle });
		upWork_->commit();

		//postLogMessage("PostgresDBClient::addHost: ok");
	}

	void PostgresDBClient::deleteHost(const std::string& host)
	{
		std::string str1{
			"DELETE FROM hosts_words "
			"WHERE host_id = "
			"(SELECT id FROM hosts "
			"WHERE host = $1);" };

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		upWork_->exec(str1, pqxx::params{ host });
		upWork_->commit();

		std::string str2{
			"DELETE FROM hosts "
			"WHERE host = $1;" };

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		upWork_->exec(str2, pqxx::params{ host });
		upWork_->commit();

		//postLogMessage("PostgresDBClient::deleteHost: ok");
	}

	void PostgresDBClient::addWords(const std::map<std::string, uint32_t>& words)
	{
		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (auto& word : words)
		{
			std::string str1{
				"INSERT INTO words(word) "
				"VALUES($1) "
				"ON CONFLICT DO NOTHING;" };

			upWork_->exec(str1, pqxx::params{ word.first });
		}
		upWork_->commit();

		//postLogMessage("PostgresDBClient::addWords: ok");
	}

	void PostgresDBClient::deleteWords(const std::map<std::string, uint32_t>& words)
	{
		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (auto& word : words)
		{
			std::string str1{
				"DELETE FROM hosts_words "
				"WHERE word_id = "
				"(SELECT id FROM words "
				"WHERE word = $1);" };

			upWork_->exec(str1, pqxx::params{ word.first });
		}
		upWork_->commit();

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (auto& word : words)
		{
			std::string str2{
				"DELETE FROM words "
				"WHERE word = $1;" };

			upWork_->exec(str2, pqxx::params{ word.first });
		}
		upWork_->commit();

		//postLogMessage("PostgresDBClient::deleteWords: ok");
	}

	void PostgresDBClient::bindWordsToHost(const std::string& host, const std::map<std::string, uint32_t>& words)
	{
		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (auto& word : words)
		{
			std::string str1{
				"INSERT INTO hosts_words(host_id, word_id, word_frequency) "
				"VALUES("
				"(SELECT id FROM hosts "
				"WHERE host = $1), "
				"(SELECT id FROM words "
				"WHERE word = $2), "
				"$3) "
				"ON CONFLICT DO NOTHING;"
		};

			upWork_->exec(str1, pqxx::params{ host, word.first, word.second });
		}
		upWork_->commit();

		//postLogMessage("PostgresDBClient::bindWordsToHost: ok");
	}

	void PostgresDBClient::addHostAndWords(const std::string& host, const std::string& hostTitle,
		const std::map<std::string, uint32_t>& words)
	{
		addHost(host, hostTitle);
		addWords(words);
		bindWordsToHost(host, words);
	}

	std::map<std::string, uint32_t> PostgresDBClient::getHostWords(const std::string& host)
	{
		std::map<std::string, uint32_t> words{};

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (const auto& [host_id, word_id, word] : upWork_->query<uint32_t, uint32_t, std::string>
			("SELECT hosts.id, words.id, words.word FROM hosts_words "
			"LEFT JOIN hosts ON hosts.id = hosts_words.host_id "
			"LEFT JOIN words ON words.id = hosts_words.word_id "
			"WHERE hosts.host = '" + host +
			"';"))
		{
			for (const auto& [word_frequency] : upWork_->query<uint32_t>
				("SELECT word_frequency FROM hosts_words "
					"WHERE host_id = " + std::to_string(host_id) +
					" AND word_id = " + std::to_string(word_id) +
					";"))
			{
				words[word] = word_frequency;
			}
		}
		upWork_->commit();

		//postLogMessage("PostgresDBClient::getHostWords: ok");
		return words;
	}

	std::map<uint32_t, uint32_t> PostgresDBClient::getWordHostsIdFrequency(const std::string& word)
	{
		std::map<uint32_t, uint32_t> ret{};

		std::string str1 = "SELECT hosts.id, words.id FROM hosts_words "
			"LEFT JOIN hosts ON hosts.id = hosts_words.host_id "
			"LEFT JOIN words ON words.id = hosts_words.word_id "
			"WHERE words.word = '" + word +
			"';";

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (const auto& [host_id, word_id] : upWork_->query<uint32_t, uint32_t>(str1))
		{
			for (const auto& [word_frequency] : upWork_->query<uint32_t>
				("SELECT word_frequency FROM hosts_words "
					"WHERE host_id = " + std::to_string(host_id) +
					" AND word_id = " + std::to_string(word_id) +
					";"))
			{
				ret[host_id] = word_frequency;
			}
		}
		upWork_->commit();

		//postLogMessage("PostgresDBClient::getWordsHostsIdFrequency: ok");
		return ret;
	}

	SearchResult PostgresDBClient::getHost(uint32_t hostId)
	{
		SearchResult ret{};

		upWork_ = std::make_unique<pqxx::work>(*upConnection_.get());
		for (const auto& [host, host_title] : upWork_->query<std::string, std::string>
			("SELECT host, host_title FROM hosts "
				"WHERE id = " + std::to_string(hostId) +
				";"))
		{
			ret.host = host;
			ret.hostTitle = host_title;
		}
		upWork_->commit();

		return ret;
	}

	std::map<uint32_t, SearchResult> PostgresDBClient::getSearchResults(const std::set<std::string>& searchWords)
	{
		std::map<uint32_t, SearchResult> ret{};       // <rating, search_result>
		std::map<uint32_t, SearchResult> tempHost{};  // <host_id, search_result>
		std::map<uint32_t, uint32_t> tempRating{};    // <host_id, rating>

		for (const auto& searchWord : searchWords)
		{
			auto hostsIdFrequency = getWordHostsIdFrequency(searchWord);
			for (const auto& [host_id, rating] : hostsIdFrequency)
			{
				tempRating[host_id] += rating;
				tempHost[host_id].searchWordsCount++;
			}
		}

		for (const auto& [host_id, rating] : tempRating)
		{
			if (tempHost[host_id].searchWordsCount == searchWords.size())
			{
				ret[rating] = getHost(host_id);
			}
		}

		return ret;
	}
}