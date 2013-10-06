/*
 * application_interface.h
 *
 *  Created on: Oct 6, 2013
 *      Author: jkrasna
 */

#ifndef APPLICATION_INTERFACE_H_
#define APPLICATION_INTERFACE_H_

#include <boost/thread.hpp>

using namespace boost;

class ApplicationInterface {
public:
	ApplicationInterface();
	virtual ~ApplicationInterface();

private:
	void worker(ApplicationInterface *);

	thread *worker_thread_;
};

#endif /* APPLICATION_INTERFACE_H_ */
