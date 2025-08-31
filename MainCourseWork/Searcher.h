#pragma once

#include "HTTPServer.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "shared.h"
#include "PostgresDBClient.h"


namespace ak
{
	class Searcher final
	{
	public:
		Searcher() = delete;
		Searcher(const GeneralState& state);
		explicit Searcher(const Searcher& obj) = delete;
		~Searcher() {}
		Searcher& operator=(const Searcher& obj) = delete;

	private:
		void runHTTPServer_();
		std::set<std::string> waitGetRequest_();
		void getPostgresDbData_(const std::set<std::string>& searchWords);
		void postResults_();

		std::unique_ptr<HTTPServer> upHTTPServer_{};

		GeneralState state_{};
	};
}