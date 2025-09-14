#pragma once

#include <pqxx/pqxx>

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>

#include "shared.h"


namespace ak
{
	class PostgresDBClient final
	{
	public:
		PostgresDBClient() = delete;
		PostgresDBClient(const PostgresDb& postgresDb);
		explicit PostgresDBClient(const PostgresDBClient& obj) = delete;
		~PostgresDBClient() {}
		PostgresDBClient& operator=(const PostgresDBClient& obj) = delete;

		void deleteTables();
		void createTables();
		void addHost(const std::string& host, const std::string& hostTitle);
		void deleteHost(const std::string& host);
		void addWords(const std::map<std::string, uint32_t>& words);
		void deleteWords(const std::map<std::string, uint32_t>& words);
		void bindWordsToHost(const std::string& host, const std::map<std::string, uint32_t>& words);
		void addHostAndWords(const std::string& host, const std::string& hostTitle, const std::map<std::string, uint32_t>& words);
		std::map<std::string, uint32_t> getHostWords(const std::string& host);
		std::map<uint32_t, uint32_t> getWordHostsIdFrequency(const std::string& word);
		SearchResult getHost(uint32_t hostId);
		std::map<uint32_t, SearchResult> getSearchResults(const std::set<std::string>& searchWords);

	private:
		std::unique_ptr<pqxx::connection> upConnection_{};
		PostgresDb postgresDb_{};
	};
}