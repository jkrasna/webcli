/*
 * utils.h
 *
 *  Created on: Nov 21, 2013
 *      Author: jkrasna
 */

#ifndef WEBCLI_UTILS_H_
#define WEBCLI_UTILS_H_

#include "shared.h"

class Utils {
public:
	Utils();
	virtual ~Utils();

	static std::string *read_file(std::string filename);
};

#endif /* WEBCLI_UTILS_H_ */
