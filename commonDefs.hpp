#ifndef COMMONDEFS_HPP 
#define COMMONDEFS_HPP

#include <iostream>

namespace term_app {

constexpr char APP_NAME[] = "term";

constexpr int LOCAL_PORT_TO_LISTEN = 30000;

constexpr std::size_t MAX_NUM_SESSIONS_ALLOWED = 3;


inline std::string formErrnoString(const char* s) {
    std::string ss{s};
    ss.append("errno = ") + std::to_string(errno) + std::string(" ");
    
    return (ss + std::strerror(errno));
}

}

#endif
