#ifndef __REQUEST_H__
#include "cs537.h"

void requestHandle(request_t request, thread_t * thread);
request_t requestParse(request_t request);

#endif
