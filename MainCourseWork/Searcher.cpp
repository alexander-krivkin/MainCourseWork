#include "Searcher.h"


namespace ak
{
	Searcher::Searcher(const GeneralState& state) : state_(state)
	{
		upHTTPServer_ = std::unique_ptr<HTTPServer>(new HTTPServer{ state_ });
	}

	void Searcher::waitAndStop()
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 2));
		std::cout << std::endl << "Application::run: нажмите Esc для остановки сервера . . ." << std::endl;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 15));

		while (true)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (GetKeyState(VK_ESCAPE) & 0x8000) { break; }
		}

		upHTTPServer_->stop();
	}
}