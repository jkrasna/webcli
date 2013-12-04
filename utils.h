#ifndef WEBCLI_UTILS_H_
#define WEBCLI_UTILS_H_

#include "shared.h"

class Utils {
public:
	static StringPtr read_file(std::string filename);
	static std::string get_iso_time();
};

#endif /* WEBCLI_UTILS_H_ */
