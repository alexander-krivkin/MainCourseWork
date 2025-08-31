#pragma once

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <variant>

#include "shared.h"


namespace ak
{
	enum class IniLoaderSyntax
	{
		eInitSection = 0,
		eInitVariable = 1,
		eErrInvalidName = 2,
		eErrNoDefinition = 3,
		eErrUnexpectedSymbols = 4
	};

	bool isLetterDigit(const char& ch);
	bool isDigit(const char& ch);
	bool isDigitDot(const char& ch);

	class IniLoader final
	{
	public:
		IniLoader() = delete;
		explicit IniLoader(const IniLoader& obj) = delete;
		explicit IniLoader(const std::string& filename);
		~IniLoader() {};
		IniLoader& operator=(const IniLoader& obj) = delete;

		template <typename T>
		T getValue(std::string query)
		{
			static_assert(sizeof(T) == -1, "IniLoader::getValue: неподдерживаемый тип данных");
		}

		template <>
		int getValue(std::string query)
		{
			return std::stoi(getStringValue_(query));
		}

		template <>
		float getValue(std::string query)
		{
			return std::stof(getStringValue_(query));
		}

		template <>
		double getValue(std::string query)
		{
			return std::stod(getStringValue_(query));
		}

		template <>
		uint32_t getValue(std::string query)
		{
			return static_cast<uint32_t>((std::stoi(getStringValue_(query))));
		}

		template <>
		std::string getValue(std::string query)
		{
			return getStringValue_(query);
		}

	private:
		void postLogRawLines_() const;
		void postLogLines_() const;
		void postLogSyntax_() const;
		void postLogContainer_() const;

		void cleanRawLines_();
		void parseData_();

		std::string getStringValue_(std::string query)
		{
			auto it = std::find(query.begin(), query.end(), '.');
			std::string section(query.begin(), it);
			std::string variable(it + 1, query.end());

			return container_[section][variable];
		}

		std::vector<std::string> rawLines_{};
		std::vector<std::string> lines_{};
		std::vector <IniLoaderSyntax> syntax_{};
		std::map<std::string, std::map<std::string, std::string>> container_{};
	};
}