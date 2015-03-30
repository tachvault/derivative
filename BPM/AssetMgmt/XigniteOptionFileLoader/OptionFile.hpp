/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_OPTIONFILE_H_
#define _DERIVATIVE_OPTIONFILE_H_

#include <string>
#include <iostream>
#include <cpprest/filestream.h>

#include "Global.hpp"

namespace derivative
{
	using namespace utility;

	class OptionFile
	{

	public:

		enum {TYPEID = INTERFACE_OPTIONFILE_TYPE};

		enum OptionType { OPTION_TYPE_UNKNOWN = 0, VANILLA_CALL = 1, VANILLA_PUT =2};

		OptionFile(const string_t& ex, OptionType opt, const dd::date& date)
			:m_exchange(ex), m_opt(opt), m_fileDate(date)
		{}
		
		~OptionFile()
		{}

		OptionType GetOptionType() const
		{
			return m_opt;
		}

		//// Return the date for this option file.
		dd::date   GetFileDate() const
		{
			return m_fileDate;
		}

		const string_t& GetExchange() const
		{
			return m_exchange;
		}

		/// return file URL
		string_t GetFileURL() const
		{
			return m_URL;
		}

		const string& GetFileName() const
		{
			return m_OptFileName;
		}

		void SetOptionType(OptionType opt)
		{
			m_opt = opt;
		}

		void SetFileDate(dd::date& date)
		{
			m_fileDate = date;
		}

		void SetExchange(const string_t& ex)
		{
			m_exchange = ex;
		}

		/// return file URL
		void SetFileURL(const string_t& url)
		{
			m_URL = url;
		}

		void SetFileName(const string& fileName)
		{
			m_OptFileName = fileName;
		}
		
	private:

		OptionType m_opt;

		string_t m_exchange;

		dd::date m_fileDate;

		string_t m_URL;

		string m_OptFileName;
	};
}


/* namespace derivative */

#endif /* _IDERIVATIVE_OPTIONFILE_H_ */