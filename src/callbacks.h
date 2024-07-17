#ifndef __CALLBACKS_H
#define __CALLBACKS_H

#include <functional>

#define SYNIO_MEMBER_FNC0(f) std::bind(&f, this)
#define SYNIO_MEMBER_FNC1(f) std::bind(&f, this, std::placeholders::_1)
#define SYNIO_MEMBER_FNC2(f) std::bind(&f, this, std::placeholders::_1, std::placeholders::_2)

typedef std::function<void(std::string)> WindowCallback;


#endif // __CALLBACKS_H
