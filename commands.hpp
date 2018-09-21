#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <map>
#include <string>

namespace term_app {

const std::string empty_cmd("");

const std::string help_cmd("help");
const std::string cmd1_cmd("cmd1");
const std::string cmd2_cmd("cmd2");
const std::string exit_cmd("exit");
const std::string quit_cmd("quit");
const std::string clear_cmd("clear");

enum class cmdCode {
    empty_input,
    notValid,

    help,
    cmd1,
    cmd2,
    exit,
    quit, 
    clear,
};

const std::map<std::string, cmdCode> cmdMap {
    {empty_cmd , cmdCode::empty_input},

    {help_cmd , cmdCode::help},
    {cmd1_cmd , cmdCode::cmd1},
    {cmd2_cmd , cmdCode::cmd2},
    {exit_cmd , cmdCode::exit},
    {quit_cmd , cmdCode::quit},
    {clear_cmd , cmdCode::clear}
};

};

#endif
