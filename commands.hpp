#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <map>

namespace term_app {

constexpr char help_cmd[] = "help";
constexpr char cmd1_cmd[] = "cmd1";
constexpr char cmd2_cmd[] = "cmd2";
constexpr char exit_cmd[] = "exit";
constexpr char quit_cmd[] = "quit";

enum class cmdCode {
    help,
    cmd1,
    cmd2,
    exit,
    quit,

    notValid,
};

const std::map<const char*, cmdCode> cmdMap {
    {help_cmd , cmdCode::help},
    {cmd1_cmd , cmdCode::cmd1},
    {cmd1_cmd , cmdCode::cmd2},
    {exit_cmd , cmdCode::exit},
    {quit_cmd , cmdCode::quit}
};

struct cmdPack {
    cmdCode code;
    std::string args;
};

enum class retCode{
    OK,
    NOK,
    EXIT,
};

};

#endif
