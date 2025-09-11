#include "Application.h"


namespace ak
{
	int Application::run()
	{
		SetConsoleCP(1251);
		SetConsoleOutputCP(1251);

		try
		{
			loadConfigFromFile_();
			runSearcher_();
			runCrawler_();
			waitAndStopCrawler_();
		}
		catch (const std::exception& ex)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 4));
			std::cout << std::endl << "Исключение: ";
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 15));
			std::cout << ex.what();

			std::cout << std::endl << std::endl;
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 2));
			system("pause");
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 15));
			std::cout << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 2));
		system("pause");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 15));
		std::cout << std::endl;
		return EXIT_SUCCESS;
	}

	void Application::loadConfigFromFile_()
	{
		upIniLoader_ = std::unique_ptr<IniLoader>(new IniLoader{ configFilename });

		state_.maxThreads = upIniLoader_->getValue<uint32_t>("main.maxThreads");
		state_.searchDepth = upIniLoader_->getValue<uint32_t>("main.searchDepth");
		state_.searchLimit = upIniLoader_->getValue<uint32_t>("main.searchLimit");		
		state_.httpsHost = upIniLoader_->getValue<std::string>("main.httpsHost");
		state_.httpsHostParams = upIniLoader_->getValue<std::string>("main.httpsHostParams");
		state_.httpsPort = upIniLoader_->getValue<std::string>("main.httpsPort");
		state_.httpServerAddress = upIniLoader_->getValue<std::string>("main.httpServerAddress");
		state_.httpServerPort = upIniLoader_->getValue<uint32_t>("main.httpServerPort");
		state_.dbHost = upIniLoader_->getValue<std::string>("main.dbHost");
		state_.dbPort = upIniLoader_->getValue<uint32_t>("main.dbPort");
		state_.dbName = upIniLoader_->getValue<std::string>("main.dbName");
		state_.dbUser = upIniLoader_->getValue<std::string>("main.dbUser");
		state_.dbPassword = upIniLoader_->getValue<std::string>("main.dbPassword");

		postLogMessage("Application::loadConfigFromFile_: конфигурация загружена");
	}

	void Application::runSearcher_()
	{
		upSearcher_ = std::unique_ptr<Searcher>(new Searcher{ state_ });
	}

	void Application::runCrawler_()
	{
		upCrawler_ = std::unique_ptr<Crawler>(new Crawler{ state_ });
	}

	void Application::waitAndStopCrawler_()
	{
		upCrawler_->waitAndStop();
	}
}