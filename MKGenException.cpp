#include "MKGenException.h"

namespace MKBasic {

MKGenException::MKGenException()
{
	msCause = "Ouch!";
}

MKGenException::MKGenException(string cause)
{
	msCause = cause;
}

string MKGenException::GetCause()
{
	return msCause;
}

} // namespace MKBasic
