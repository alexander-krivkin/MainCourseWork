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
	constexpr auto userAgentName{ "Mozilla / 5.0 (X11; Linux x86_64; rv:86.0) Gecko / 20100101 Firefox / 86.0" };
	constexpr uint32_t workTimeout{ 1000 };
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
			return ((httpsHost <= obj.httpsHost) ?
				(httpsHostParams < obj.httpsHostParams) : false);
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

	inline std::string toLower(const std::string& str)
	{
		if (str == "") { return ""; }

		std::string ret{ str };

		// 1. Латинница
		std::transform(ret.begin(), ret.end(), ret.begin(),
			[](char ch) { return std::tolower(ch); }
		);

		// 2. Кириллица
		std::map<char, char> trans
		{
			{ 'А', 'а' },
			{ 'Б', 'б' },
			{ 'В', 'в' },
			{ 'Г', 'г' },
			{ 'Д', 'д' },
			{ 'Е', 'е' },
			{ 'Ё', 'ё' },
			{ 'Ж', 'ж' },
			{ 'З', 'з' },
			{ 'И', 'и' },
			{ 'Й', 'й' },
			{ 'К', 'к' },
			{ 'Л', 'л' },
			{ 'М', 'м' },
			{ 'Н', 'н' },
			{ 'О', 'о' },
			{ 'П', 'п' },
			{ 'Р', 'р' },
			{ 'С', 'с' },
			{ 'Т', 'т' },
			{ 'У', 'у' },
			{ 'Ф', 'ф' },
			{ 'Х', 'х' },
			{ 'Ц', 'ц' },
			{ 'Ч', 'ч' },
			{ 'Ш', 'ш' },
			{ 'Щ', 'щ' },
			{ 'Ь', 'ь' },
			{ 'Ы', 'ы' },
			{ 'Ъ', 'ъ' },
			{ 'Э', 'э' },
			{ 'Ю', 'ю' },
			{ 'Я', 'я' }
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
			{ 'а' },
			{ 'б' },
			{ 'в' },
			{ 'г' },
			{ 'д' },
			{ 'е' },
			{ 'ё' },
			{ 'ж' },
			{ 'з' },
			{ 'и' },
			{ 'й' },
			{ 'к' },
			{ 'л' },
			{ 'м' },
			{ 'н' },
			{ 'о' },
			{ 'п' },
			{ 'р' },
			{ 'с' },
			{ 'т' },
			{ 'у' },
			{ 'ф' },
			{ 'х' },
			{ 'ц' },
			{ 'ч' },
			{ 'ш' },
			{ 'щ' },
			{ 'ь' },
			{ 'ы' },
			{ 'ъ' },
			{ 'э' },
			{ 'ю' },
			{ 'я' }
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

	inline std::string findAndReplaceRegex(const std::string& str, const std::string& regex,
		const std::string& replaceStr)
	{
		if (str == "") { return ""; }

		return std::regex_replace(str, std::regex(regex), replaceStr);
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