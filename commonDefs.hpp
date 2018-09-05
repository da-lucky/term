#ifndef COMMONDEFS_HPP 
#define COMMONDEFS_HPP

#include <iostream>
#include <sstream>


namespace term_app {

constexpr char APP_NAME[] = "term";

constexpr int LOCAL_PORT_TO_LISTEN = 30000;

constexpr std::size_t MAX_NUM_SESSIONS_ALLOWED = 3;
    
constexpr std::size_t MAX_BUF_SIZE = 1;

constexpr std::size_t CTRL_CMD_SIZE = 3;

inline std::string formErrnoString(const char* s) {
    std::stringstream ss{s};
    ss << " errno = " << errno << " " << std::strerror(errno);
    return ss.str();
}

constexpr char IAC = '\xff';

const std::string SPACE_TAB{'\x20','\x09'};

const std::string CR_LF{'\xd','\xa'};


}

#endif
