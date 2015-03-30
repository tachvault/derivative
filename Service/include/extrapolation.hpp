/*
 Copyright (C) Nathan Muruganantha 2015
 Copyright (C) 2004 StatPro Italia srl
 */

/// class-wide extrapolation settings

#ifndef _DERIVATIVE_EXTRAPOLATION_H
#define _DERIVATIVE_EXTRAPOLATION_H

namespace derivative
{
    //! base class for classes possibly allowing extrapolation
    class Extrapolator 
	{
      public:
        Extrapolator() 
			: extrapolate_(false) 
		{}
        virtual ~Extrapolator() 
		{}
        //! \name modifiers
        //@{
        //! enable extrapolation in subsequent calls
        void enableExtrapolation(bool b = true) 
		{
			extrapolate_ = b; 
		}
        //! disable extrapolation in subsequent calls
        void disableExtrapolation(bool b = true) 
		{ 
			extrapolate_ = !b; 
		}
        //@}
        //! \name inspectors
        //@{
        //! tells whether extrapolation is enabled
        bool allowsExtrapolation() const 
		{
			return extrapolate_;
		}
        //@}
      protected:
        bool extrapolate_;
    };
}

/* namespace derivative */
#endif /* _DERIVATIVE_EXTRAPOLATION_H */
