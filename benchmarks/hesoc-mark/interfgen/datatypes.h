#ifndef _DTYPES_
#define _DTYPES_

#include <string>

enum ProcType
{
	UT,
	IF
};

struct testitem_t {

ProcType procType;
std::string cmdLine;
std::string argsLine;
std::string cores;
std::string engine;

};


struct interfgenArgs {
	bool verbosity;
	bool help;
	std::string outdir;
	std::string infile;
};

#endif
