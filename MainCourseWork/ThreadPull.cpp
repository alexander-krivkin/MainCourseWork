#include "ThreadPull.h"


namespace ak
{
	ThreadPull::ThreadPull(uint32_t threadsCount)
	{
		std::stringstream strS{};
		strS << "ThreadPull::ThreadPull: пул потоков запущен, ширина " << threadsCount << " потока";
		postLogMessage(strS.str());

		stopped_ = false;
		for (uint32_t idx{}; idx < threadsCount; idx++)
		{
			threads_.emplace_back([this]() { work_(); });
		}
	}

	ThreadPull::~ThreadPull()
	{
		if (!stopped_) stop();
	}

	void ThreadPull::submit(UniqueFunction function)
	{
		safeThreadQueue_.push(std::move(function));
	}

	void ThreadPull::stop()
	{
		stopped_ = true;
		for (auto& thread : threads_)
		{
			thread.detach();
		}
		
		postLogMessage("ThreadPull::stop: пул потоков остановлен");
	}

	void ThreadPull::work_()
	{
		while (true)
		{
			auto func = safeThreadQueue_.pop();

			//std::unique_lock<std::mutex> ulMtx(mtx_);
			//ulMtx.unlock();

			func();
			workCount_++;

			if (stopped_) { break; }
		}
	}
}