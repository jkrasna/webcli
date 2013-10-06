#ifndef WEB_INTERFACE_H_
#define WEB_INTERFACE_H_

#include <iostream>

class ApplicationInterface;

class WebInterface {
public:
	WebInterface(ApplicationInterface *);
	virtual ~WebInterface(void);

	void process(void);
private:
	ApplicationInterface * application_interface_;
};

#endif /* WEB_INTERFACE_H_ */
