#include "Searcher.h"


namespace ak
{
	Searcher::Searcher(const GeneralState& state) : state_(state)
	{
		runHTTPServer_();

		std::stringstream strS{};
		strS << "Searcher::Searcher: поисковик запущен";
		postLogMessage(strS.str());

		auto searchWords = waitGetRequest_();
		getPostgresDbData_(searchWords);
		postResults_();
	}

	void Searcher::runHTTPServer_()
	{
		upHTTPServer_ = std::unique_ptr<HTTPServer>(new HTTPServer{
			{ state_.httpServerAddress, state_.httpServerPort } });
	}

	std::set<std::string> Searcher::waitGetRequest_()
	{
		return {};
	}

	void Searcher::getPostgresDbData_(const std::set<std::string>& searchWords)
	{

	}

	void Searcher::postResults_()
	{

	}
}