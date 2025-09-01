#include "Searcher.h"


namespace ak
{
	Searcher::Searcher(const GeneralState& state) : state_(state)
	{
		runHTTPServer_();

		std::stringstream strS{};
		strS << "Searcher::Searcher: поисковик запущен";
		postLogMessage(strS.str());
	}

	void Searcher::runHTTPServer_()
	{
		upHTTPServer_ = std::unique_ptr<HTTPServer>(new HTTPServer{ state_ });
	}
}