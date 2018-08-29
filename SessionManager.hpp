#ifndef SESSION_MANAGER_HPP
#define SESSION_MANAGER_HPP

#include <unistd.h>
#include <sys/socket.h>
#include <cstring>
#include <iostream>
#include <array>
#include <thread>
#include <memory>
#include <algorithm>
#include "session.hpp"
#include "commonDefs.hpp"

namespace term_app {

session::SessionStatus startSession(int socket);

}

#endif
