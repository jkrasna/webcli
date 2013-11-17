#ifndef WEBCLI_LOGGING_H_
#define WEBCLI_LOGGING_H_

#include "shared.h"
#include "log_sink.h"

#define	LL_NO 			0	/* No logging level */
#define	LL_CRITICAL		1	/* Critical log level */
#define LL_ERROR		2	/* Error log level */
#define LL_WARNING		3	/* Warning log level */
#define LL_DEBUG		4	/* Debug log level */
#define LL_TRACE		5	/* Trace log level */
#define LL_ALL			6	/* All logging level */

#define LOG_EXPAND(level, args...) 	LOG_CALL(level, __FILE__, __LINE__, args)
#define LOG_CALL(level, file, line, args...) \
do{\
	const size_t __buffer_size = 255;\
	char __buffer[__buffer_size];\
	snprintf(&__buffer[0], __buffer_size, "%s'%s:%d': ", Logging::get_level_string(level), file, line);\
	std::string __message(__buffer);\
	snprintf(&__buffer[0], __buffer_size, args);\
	__message.append(__buffer);\
	Logging::log(level, __message);\
}while(0)

#define LOG_CRT(args...) LOG_EXPAND(LL_CRITICAL, args)
#define LOG_ERR(args...) LOG_EXPAND(LL_ERROR, args)
#define LOG_WRN(args...) LOG_EXPAND(LL_WARNING, args)
#define LOG_DBG(args...) LOG_EXPAND(LL_DEBUG, args)
#define LOG_TRC(args...) LOG_EXPAND(LL_TRACE, args)

typedef std::unique_ptr<LogSink> LogSinkPtr;

class Logging {
public:
	static Logging& get_instance()
	{
		static Logging instance;
		return instance;
	}

	static void add_sink(LogSink *sink);

	static void log(int level, std::string message);

	static const char *get_level_string(int level);

private:
	Logging();
	Logging(Logging const&);            // Don't Implement
	void operator=(Logging const&); 	// Don't implement
	virtual ~Logging(void);

	std::vector<LogSinkPtr> *sink_list_;
	std::mutex *mutex_;
};

#endif /* WEBCLI_LOGGING_H_ */
