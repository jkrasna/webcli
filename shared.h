#ifndef WEBCLI_PRECOMPILED_H_
#define WEBCLI_PRECOMPILED_H_

#include <string>
#include <vector>
#include <deque>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <algorithm>

#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#define MILI_TO_MICRO(mili) 	mili*1000

typedef std::shared_ptr<std::string> StringPtr;
typedef std::shared_ptr<std::stringstream> StringStreamPtr;

#endif /* WEBCLI_PRECOMPILED_H_ */
