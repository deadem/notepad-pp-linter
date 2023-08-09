#pragma once

#include <string>
#include <vector>

class XmlParser
{
public:
	struct Error
	{
		int m_line;
		int m_column;
		std::wstring m_message;
	};

	struct Linter
	{
		std::wstring m_extension;
		std::wstring m_command;
		bool m_useStdin = false;
	};

	struct Settings {
		Settings() : m_alpha(-1), m_color(-1)
		{
		}

		int m_color;
		int m_alpha;
		std::vector<XmlParser::Linter> m_linters;
	};

	static std::vector<Error> getErrors(const std::string& xml);
	static Settings getLinters(std::wstring file);
};
