#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <string>

#define TOOLNAME "HESOCMARK"

#define PRINT_RESULT(x,y){\
std::string _ts(TOOLNAME);\
_ts.append(": %d\t%#.3g\n");\
fprintf(stdout,_ts.c_str(),x,y);\
}

#define PRINT_RESULT2(x){\
std::string _ts(TOOLNAME);\
_ts.append(" %0.6lf\n");\
fprintf(stdout,_ts.c_str(),x);\
}

#endif /*_COMMON_H_*/
