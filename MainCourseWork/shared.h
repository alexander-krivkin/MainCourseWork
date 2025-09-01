#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <future>
#include <chrono>
#include <algorithm>
#include <regex>
#include <windows.h>


namespace ak
{
	constexpr auto configFilename{ "config.ini" };
	constexpr uint32_t workTimeout{ 5000 };
	constexpr uint32_t deadlockTimeout{ 5000 };

	struct GeneralState
	{
		uint32_t maxThreads{};
		uint32_t searchDepth{};
		uint32_t searchLimit{};
		std::string httpsHost{};
		std::string httpsHostParams{};
		std::string httpsPort{};
		std::string httpServerAddress{};
		uint32_t httpServerPort{};
		std::string dbHost{};
		uint32_t dbPort{};
		std::string dbName{};
		std::string dbUser{};
		std::string dbPassword{};
	};

	struct PostgresDb
	{
		std::string dbHost{};
		uint32_t dbPort{};
		std::string dbName{};
		std::string dbUser{};
		std::string dbPassword{};
	};

	struct Host
	{
		uint32_t level{};
		std::string httpsHost{};
		std::string httpsHostParams{};
		std::string httpsPort{};

		bool operator <(const Host& obj) const
		{
			if (httpsHost != obj.httpsHost)
			{
				return (httpsHost < obj.httpsHost);
			}
			else
			{
				return (httpsHostParams < obj.httpsHostParams);
			}
		}
	};

	struct Server
	{
		std::string httpServerAddress{};
		uint32_t httpServerPort{};
	};

	struct IndexedHostData
	{
		uint32_t level{};
		std::string httpsHostWithParams{};
		std::string indexedTitle{};
		std::set<Host> indexedHosts{};
		std::map<std::string, uint32_t> indexedWords{};
	};

	struct SearchResult
	{
		std::string host{};
		std::string hostTitle{};
		uint32_t searchWordsCount{};
	};

	inline std::string utf8ToCp1251(std::string const& str)
	{
		if (str.empty())
		{
			return { "" };
		}

		size_t wchlen = static_cast<size_t>(MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
			static_cast<int>(str.size()), NULL, 0));
		if (wchlen > 0 && wchlen != 0xFFFD)
		{
			std::vector<wchar_t> wbuf(wchlen);
			MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()),
				&wbuf[0], static_cast<int>(wchlen));
			std::vector<char> buf(wchlen);
			WideCharToMultiByte(1251, 0, &wbuf[0], static_cast<int>(wchlen), &buf[0],
				static_cast<int>(wchlen), 0, 0);

			return std::string{ &buf[0], wchlen };
		}
		else
		{
			return { "" };
		}
	}

	inline std::string findAndReplaceRegex(const std::string& str, const std::string& regex,
		const std::string& replaceStr)
	{
		if (str == "") { return ""; }

		return std::regex_replace(str, std::regex(regex), replaceStr);
	}

	inline std::string toLower(const std::string& str)
	{
		if (str == "") { return ""; }

		std::string ret{ str };

		// 1. ���������
		std::transform(ret.begin(), ret.end(), ret.begin(),
			[](char ch) { return std::tolower(ch); }
		);

		// 2. ���������
		std::map<char, char> trans
		{
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' },
			{ '�', '�' }
		};

		std::transform(ret.begin(), ret.end(), ret.begin(),
			[trans](char ch)
			{
				for (const auto& [upper, lower] : trans)
				{
					if (ch == upper) { return lower; }
				}
				return ch;
			}
		);

		return ret;
	}

	inline std::string toCyrillicWords(const std::string& str)
	{
		if (str == "") { return ""; }

		std::string ret{ str };

		std::set<int> trans
		{
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' },
			{ '�' }
		};

		std::transform(ret.begin(), ret.end(), ret.begin(),
			[trans](int ch)
			{
				for (const auto& elem : trans)
				{
					if (ch == elem) { return elem; }
				}
				return static_cast<int>(' ');
			}
		);

		return ret;
	}

	inline std::string urlDecode(const std::string& str)
	{
		if (str == "") { return ""; }

		std::string ret{ str };

		std::map<std::string, std::string> trans
		{
			{ "%C0", "�" },
			{ "%C1", "�" },
			{ "%C2", "�" },
			{ "%C3", "�" },
			{ "%C4", "�" },
			{ "%C5", "�" },
			{ "%A8", "�" },
			{ "%C6", "�" },
			{ "%C7", "�" },
			{ "%C8", "�" },
			{ "%C9", "�" },
			{ "%CA", "�" },
			{ "%CB", "�" },
			{ "%CC", "�" },
			{ "%CD", "�" },
			{ "%CE", "�" },
			{ "%CF", "�" },
			{ "%D0", "�" },
			{ "%D1", "�" },
			{ "%D2", "�" },
			{ "%D3", "�" },
			{ "%D4", "�" },
			{ "%D5", "�" },
			{ "%D6", "�" },
			{ "%D7", "�" },
			{ "%D8", "�" },
			{ "%D9", "�" },
			{ "%DC", "�" },
			{ "%DB", "�" },
			{ "%DA", "�" },
			{ "%DD", "�" },
			{ "%DE", "�" },
			{ "%DF", "�" },
			{ "%E0", "�" },
			{ "%E1", "�" },
			{ "%E2", "�" },
			{ "%E3", "�" },
			{ "%E4", "�" },
			{ "%E5", "�" },
			{ "%B8", "�" },
			{ "%E6", "�" },
			{ "%E7", "�" },
			{ "%E8", "�" },
			{ "%E9", "�" },
			{ "%EA", "�" },
			{ "%EB", "�" },
			{ "%EC", "�" },
			{ "%ED", "�" },
			{ "%EE", "�" },
			{ "%EF", "�" },
			{ "%F0", "�" },
			{ "%F1", "�" },
			{ "%F2", "�" },
			{ "%F3", "�" },
			{ "%F4", "�" },
			{ "%F5", "�" },
			{ "%F6", "�" },
			{ "%F7", "�" },
			{ "%F8", "�" },
			{ "%F9", "�" },
			{ "%FC", "�" },
			{ "%FB", "�" },
			{ "%FA", "�" },
			{ "%FD", "�" },
			{ "%FE", "�" },
			{ "%FF", "�" }
		};

		for (const auto& [coded, decoded] : trans)
		{
			ret = findAndReplaceRegex(ret, coded, decoded);
		}

		return ret;
	}

	inline std::map<std::string, uint32_t> toCounterMap(std::vector<std::string> obj)
	{
		std::map<std::string, uint32_t> ret{};

		for (auto& elem : obj)
		{
			if (ret.count(elem)) {
				ret[elem]++;
			}
			else {
				ret[elem] = 1;
			}
		}

		return ret;
	}

	inline void postLogMessage(const std::string& msg)
	{
#ifdef _DEBUG
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 14));
		std::cout << ">> ";
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ((0 << 4) | 15));
		std::cout << msg << std::endl;
#endif // _DEBUG
	}

	//using UniqueFunction = std::packaged_task<void()>;
	using UniqueFunction = std::function<void()>;
}