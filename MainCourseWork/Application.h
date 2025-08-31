#pragma once

#include "Crawler.h"
#include "Searcher.h"

#include <iostream>
#include <string>
#include <Windows.h>

#include "shared.h"
#include "IniLoader.h"


namespace ak
{
	class Application final
	{
	public:
		Application() {}
		explicit Application(const Application& obj) = delete;
		~Application() {}
		Application& operator=(const Application& obj) = delete;

		int run();

	private:
		void loadConfigFromFile_();
		void runCrawler_();
		void waitAndStopCrawler_();
		void runSearcher_();

		std::unique_ptr<IniLoader> upIniLoader_{};
		std::unique_ptr<Crawler> upCrawler_{};
		std::unique_ptr<Searcher> upSearcher_{};
		GeneralState state_{};
	};
}