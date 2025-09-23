#pragma once

#include "HTTPServer.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "shared.h"


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

		void waitAndStop();

	private:
		std::unique_ptr<HTTPServer> upHTTPServer_{};

		GeneralState state_{};
	};
}