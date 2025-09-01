#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <atomic>
#include <functional>
#include <future>
#include <chrono>

#include "shared.h"
#include "SafeThreadQueue.h"


namespace ak
{
	class ThreadPull final
	{
	public:
		ThreadPull() = delete;
		explicit ThreadPull(uint32_t threadsCount = 1);
		explicit ThreadPull(const ThreadPull& obj) = delete;
		~ThreadPull();
		ThreadPull& operator=(const ThreadPull& obj) = delete;

		void submit(UniqueFunction function);
		void stop();

	private:
		void work_();

		std::vector<std::jthread> threads_{};
		SafeThreadQueue<UniqueFunction> safeThreadQueue_{};

		std::atomic<uint32_t> workCount_{};
		std::atomic<bool> stopped_{ true };
	};
}