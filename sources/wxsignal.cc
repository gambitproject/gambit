//#
//# FILE: wxsig.cc -- Definition of signal handler for the GUI
//#
//#  $Id$
//#

#include "gsignal.h"

class WxSignal : public gSignal   {
	private:
		bool sig;

	public:
		WxSignal(void): sig(false) { };
		virtual ~WxSignal() { };

		bool Get(void) const {return sig;}
		void Reset(void) {sig=false;}
};

WxSignal _gbreak;
gSignal &gbreak = _gbreak;


