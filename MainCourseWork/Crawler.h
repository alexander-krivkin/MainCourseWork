#pragma once

#include "HTTPSClient.h"

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <atomic>

#include "shared.h"
#include "ThreadPull.h"
#include "PostgresDBClient.h"


namespace ak
{
	class Crawler final
	{
	public:
		Crawler() = delete;
		Crawler(const GeneralState& state);
		explicit Crawler(const Crawler& obj) = delete;
		~Crawler();
		Crawler& operator=(const Crawler& obj) = delete;

		void parseHost(const Host host);
		void waitAndStop();
		void stop();

	private:

		void runThreadPull_();
		void stopThreadPull_();
		void recreatePostgresDbTables_();
		std::string downloadHostData_(const Host host);
		IndexedHostData indexHostData_(const Host host, const std::string& downloadedHostData);
		void insertPostgresDbIndexedHostData_(const IndexedHostData& indexedHostData);

		std::unique_ptr<ThreadPull> upThreadPull_{};

		std::atomic<uint32_t> hostCount_;
		std::atomic<bool> stopped_;
		GeneralState state_{};
	};
}