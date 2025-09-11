#include "Searcher.h"


namespace ak
{
	Searcher::Searcher(const GeneralState& state) : state_(state)
	{
		upThread_ = std::unique_ptr<std::jthread>(
			new std::jthread(std::bind(&Searcher::runHTTPServer_, this)));
		// postLogMessage("Searcher::Searcher: поисковик запущен");
	}

	void Searcher::runHTTPServer_()
	{
		upHTTPServer_ = std::unique_ptr<HTTPServer>(new HTTPServer{ state_ });
	}
}