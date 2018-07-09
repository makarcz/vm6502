#ifndef MKGENEXCEPTION_H
#define MKGENEXCEPTION_H

#include <string>
#include <exception>

using namespace std;

namespace MKBasic {

class MKGenException : public exception {
	public:
		MKGenException();
		MKGenException(string cause);
		~MKGenException() throw() {};
		string GetCause();

	private:
		string msCause;
};

} // namespace MKBasic

#endif
