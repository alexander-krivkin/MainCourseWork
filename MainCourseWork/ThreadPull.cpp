#include "ThreadPull.h"


namespace ak
{
	ThreadPull::ThreadPull(uint32_t threadsCount)
	{
		std::stringstream strS{};
		strS << "ThreadPull::ThreadPull: ��� ������� �������, ������ " << threadsCount << " ������";
		postLogMessage(strS.str());

		stopped_.store(false);
		for (uint32_t idx{}; idx < threadsCount; idx++)
		{
			threads_.emplace_back([this]() { work_(); });
		}
	}

	ThreadPull::~ThreadPull()
	{
		if (!stopped_.load()) stop();
	}

	void ThreadPull::submit(UniqueFunction function)
	{
		safeThreadQueue_.push(std::move(function));
	}

	void ThreadPull::stop()
	{
		stopped_.store(true);
		for (auto& thread : threads_)
		{
			thread.detach();
		}
		
		postLogMessage("ThreadPull::stop: ��� ������� ����������");
	}

	void ThreadPull::work_()
	{
		while (true)
		{
			auto func = safeThreadQueue_.pop();
			func();

			workCount_++;

			if (stopped_.load()) { break; }
		}
	}
}