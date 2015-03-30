/*
Copyright (c) 2013 - 2014, Nathan Muruganantha. All rights reserved.
*/

#ifndef _DERIVATIVE_OPTIONXIGNITEDAO_H_
#define _DERIVATIVE_OPTIONXIGNITEDAO_H_

#include <ppltasks.h>
#include <memory>
#include "OptionFile.hpp"
#include "Global.hpp"
#include "DException.hpp"

namespace derivative
{
	// Data Access Object for Option entity.
	class OptionXigniteDAO
	{
	public:

		enum { TYPEID = CLASS_OPTIONXIGNITEDAO_TYPE };

		OptionXigniteDAO(OptionFile& opt)
			:m_optFile(opt)
		{
		}

		void GetFile()
		{
			m_optFile;
		}

		/// Get the option file URL
		Concurrency::task_status RetriveOptFileName();

		/// download the option file
		void RetriveOptFile();

	private:

		OptionFile m_optFile;
	};
}

/* namespace derivative */

#endif /* _DERIVATIVE_OPTIONXIGNITEDAO_H_ */
