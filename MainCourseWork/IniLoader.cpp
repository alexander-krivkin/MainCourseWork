#include "IniLoader.h"


namespace ak
{
	std::vector<std::string> syntaxName{ "ok: инициализаци€ секции", "ok: инициализаци€ переменной",
		"ошибка: некорректное им€", "ошибка: отсутствует значение", "ошибка: нераспознаваемые символы" };

	bool isLetterDigit(const char& ch) {
		return (((ch >= 'A') && (ch <= 'Z')) ||
			((ch >= 'a') && (ch <= 'z')) || ((ch >= '1') && (ch <= '9')));
	}
	bool isDigit(const char& ch) { return ((ch >= '0') && (ch <= '9')); }
	bool isDigitDot(const char& ch) { return (((ch >= '0') && (ch <= '9')) || (ch == '.')); }

	IniLoader::IniLoader(const std::string& filename)
	{
		std::ifstream file(filename);
		if (!file)
		{
			throw(std::exception("IniLoader::IniLoader: ошибка открыти€ файла"));
		}

		std::string line{};
		while (std::getline(file, line))
		{
			rawLines_.emplace_back(line);
		}

		cleanRawLines_();
		parseData_();
		postLogContainer_();
	}

	void IniLoader::postLogRawLines_() const
	{
		std::stringstream strS{};
		strS << "IniLoader::postLogRawLines_: " << std::endl;

		int idx{};
		for (const auto& elem : rawLines_)
		{
			strS << std::setw(3) << ++idx << ": " << '\"' << elem << '\"' << std::endl;
		}
		postLogMessage(strS.str());
	}

	void IniLoader::postLogLines_() const
	{
		std::stringstream strS{};
		strS << "IniLoader::postLogLines_: " << std::endl;

		int idx{};
		for (const auto& elem : lines_)
		{
			strS << std::setw(3) << ++idx << ": " << '\"' << elem << '\"' << std::endl;
		}
		postLogMessage(strS.str());
	}

	void IniLoader::postLogSyntax_() const
	{
		std::stringstream strS{};
		strS << "IniLoader::postLogSyntax_: " << std::endl;

		int idx{};
		for (const auto& elem : syntax_)
		{
			strS << std::setw(3) << ++idx << ": " << syntaxName[static_cast<int>(elem)] << std::endl;
		}
		postLogMessage(strS.str());
	}

	void IniLoader::postLogContainer_() const
	{
		std::stringstream strS{};
		strS << "IniLoader::postLogContainer_: " << std::endl;

		for (const auto& key : container_)
		{
			strS << "[" << key.first << "]" << std::endl;
			for (const auto& elem : key.second)
			{
				strS << "    " << elem.first << " = " << elem.second << std::endl;
			}
		}
		postLogMessage(strS.str());
	}

	void IniLoader::cleanRawLines_()
	{
		// 1. ”даление комментариев
		for (const auto& elem : rawLines_)
		{
			auto it = std::find(elem.begin(), elem.end(), ';');
			lines_.emplace_back(std::string(elem.begin(), it));
		}

		// 2. ”даление пробелов и символов табул€ции до знаков [
		for (auto& elem : lines_)
		{
			auto it = std::find(elem.begin(), elem.end(), '[');
			if (it != elem.end())
			{
				elem.erase(std::remove_if(elem.begin(), it,
					[](char ch) { return ((ch == ' ') || (ch == '\t')); }),
					it);
			}
		}

		// 3. ”даление пробелов и символов табул€ции после знаков ]
		for (auto& elem : lines_)
		{
			auto it = std::find(elem.begin(), elem.end(), ']');
			if (it != elem.end())
			{
				elem.erase(std::remove_if(it, elem.end(),
					[](char ch) { return ((ch == ' ') || (ch == '\t')); }),
					elem.end());
			}
		}

		// 4. ”даление пробелов и символов табул€ции до знаков =
		for (auto& elem : lines_)
		{
			auto it = std::find(elem.begin(), elem.end(), '=');
			if (it != elem.end())
			{
				elem.erase(std::remove_if(elem.begin(), it,
					[](char ch) { return ((ch == ' ') || (ch == '\t')); }),
					it);
			}
		}

		// 5. ”даление пробелов и символов табул€ции после знаков = до любого символа
		for (auto& elem : lines_)
		{
			auto it = std::find(elem.begin(), elem.end(), '=');
			if (it != elem.end())
			{
				it++;
				elem.erase(it, std::find_if(it, elem.end(),
					[](char ch) { return ((ch != ' ') && (ch != '\t')); }));
			}
		}

		// 6. ”даление завершающих строки пробелов и символов табул€ции (кодова€ страница 1251)
		for (auto& elem : lines_)
		{
			elem.erase(std::find_if(elem.rbegin(), elem.rend(),
				[](char ch) { return ((ch != ' ') && (ch != '\t')); }).base(),
				elem.end());
		}

		// 7. ”даление пустых строк
		std::erase(lines_, "");

		postLogMessage("IniLoader::cleanRawLines_: очистка строк выполнена");
	}

	void IniLoader::parseData_()
	{
		std::string currentSection{ "unnamed" };
		std::string currentVariable{};
		std::string currentVariableValue{};

		syntax_.resize(lines_.size());
		std::fill(syntax_.begin(), syntax_.end(), IniLoaderSyntax::eErrNoDefinition);
		int syntaxIdx{};

		for (const auto& elem : lines_)
		{
			// —екции
			auto lines_it = std::find(elem.begin(), elem.end(), '[');
			auto lines_it_end = std::find(lines_it, elem.end(), ']');
			if ((lines_it != elem.end()) && (lines_it_end != elem.end()))
			{
				if (std::find_if_not(lines_it + 1, lines_it_end, isLetterDigit) == lines_it_end)
				{
					syntax_[syntaxIdx] = IniLoaderSyntax::eInitSection;
					currentSection = std::string(lines_it + 1, lines_it_end);

					if (!container_[currentSection].size())
					{
						container_[currentSection].clear();
					}
				}
				else
				{
					syntax_[syntaxIdx] = IniLoaderSyntax::eErrInvalidName;
				}
			}

			// ѕеременные
			lines_it = std::find(elem.begin(), elem.end(), '=');
			if (lines_it != elem.end())
			{
				if (std::find_if_not(elem.begin(), lines_it, isLetterDigit) == lines_it)
				{
					currentVariable = std::string(elem.begin(), lines_it);
				}
				else
				{
					syntax_[syntaxIdx] = IniLoaderSyntax::eErrInvalidName;
				}

				currentVariableValue = std::string(lines_it + 1, elem.end());
				if (currentVariableValue.length())
				{
					syntax_[syntaxIdx] = IniLoaderSyntax::eInitVariable;
					container_[currentSection][currentVariable] = currentVariableValue;
				}
				else
				{
					syntax_[syntaxIdx] = IniLoaderSyntax::eErrNoDefinition;
				}
			}

			syntaxIdx++;
		}

		postLogMessage("IniLoader::parseData_: парсинг данных выполнен");
	}
}