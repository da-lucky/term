#ifndef COMMONDEFS_HPP 
#define COMMONDEFS_HPP

#include <iostream>
#include <sstream>
#include <list>

namespace term_app {

constexpr char APP_NAME[] = "term";

constexpr int LOCAL_PORT_TO_LISTEN = 30000;

constexpr std::size_t MAX_NUM_SESSIONS_ALLOWED = 3;
    
constexpr std::size_t MAX_BUF_SIZE = 64;

constexpr std::size_t CTRL_CMD_SIZE = 3;

constexpr std::size_t CTRL_CMD_BUFFER_SIZE = 12; // IAC, SB, OPT ... , IAC, SE

constexpr std::size_t MAX_USER_CMD_SIZE = MAX_BUF_SIZE;

inline std::string formErrnoString(const char* s) {
    std::stringstream ss{s};
    ss << " errno = " << errno << " " << std::strerror(errno);
    return ss.str();
}

namespace ASCII {
    constexpr char NUL {'\x00'};
    constexpr char ETX {'\x03'};
    constexpr char EOT {'\x04'};
    constexpr char ESC {'\x1b'};
    constexpr char CR {'\x0d'};
    constexpr char LF {'\x0a'};

    constexpr char SPACE {'\x20'};
    constexpr char TAB {'\x09'};
    constexpr char DEL {'\x7f'};

    const std::string CR_LF{ASCII::CR, ASCII::LF};
    const std::string CR_LF_TAB{ASCII::CR, ASCII::LF, ASCII::TAB};
    const std::string SPACE_TAB{ASCII::SPACE, ASCII::TAB};
}

namespace MOVEMENT_ESCAPE_SEQ {
    const std::string BUTTON_UP        = {'\x1b', '\x5b', '\x41'};  // \033[A
    const std::string BUTTON_DOWN      = {'\x1b', '\x5b', '\x42'};  // \033[B
    const std::string BUTTON_BW        = {'\x1b', '\x5b', '\x44'};  // \033[D
    const std::string BUTTON_FW        = {'\x1b', '\x5b', '\x43'};  // \033[C
    const std::size_t BUTTON_CODE_SIZE = 3;

    const std::string BW_FW = {'\x1b', '\x5b', '\x44', '\x1b', '\x5b', '\x43'};   // \033[D\033[C
    const std::string BW_ERASE = {'\x1b', '\x5b', '\x44', '\x1b', '\x5b', '\x4b'}; // \033[D\033[K
    const std::string CLEAR_SCR = {'\x1b', '\x5b', '\x32', '\x4a' }; // Clear the screen, move to (0,0)
    const std::string POS_0_0 = { '\x1b', '\x5b', '\x30',   '\x3b', '\x30', '\x48'};
    const std::string ERASE = {'\x1b', '\x5b', '\x4b'}; // Erase to end of line from current position 

    const std::string BLANK_LINE = std::string(1,'\r') + ERASE;
}

namespace TELNET {
namespace CMD_CODE {    
    constexpr char SE =     '\xf0';

    constexpr char SB =     '\xfa';
    constexpr char WILL =   '\xfb';
    constexpr char WONT =   '\xfc';
    constexpr char DO =     '\xfd';    
    constexpr char DONT =   '\xfe';
    constexpr char IAC =    '\xff';
}

namespace OPTS_CODE {
    constexpr char ECHO =   '\x01';
    constexpr char SGA =    '\x03';
    constexpr char NAWS =   '\x1f';
}

const std::string WILL_SGA = {CMD_CODE::IAC, CMD_CODE::WILL, OPTS_CODE::SGA};
const std::string DO_SGA = {CMD_CODE::IAC, CMD_CODE::DO, OPTS_CODE::SGA};
const std::string WILL_ECHO = {CMD_CODE::IAC, CMD_CODE::WILL, OPTS_CODE::ECHO};
const std::string DO_ECHO = {CMD_CODE::IAC, CMD_CODE::DO, OPTS_CODE::ECHO};

}

using StringList = std::list<std::string>;
}

#endif
