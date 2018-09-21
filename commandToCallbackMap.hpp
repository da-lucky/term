#ifndef CMD2CBMAP
#define CMD2CBMAP

#include <map>
#include <functional>
#include "commands.hpp"
#include "help.hpp"

namespace term_app {

using callback = std::function<std::string(const std::string&)>;

struct funcCbDescriptor {
    std::string name;
    callback cb;
};

const std::map<cmdCode, funcCbDescriptor> fCbMap {
    {cmdCode::empty_input,  {empty_cmd, [](const std::string&) { return std::string {}; }}},
    {cmdCode::notValid,     {empty_cmd, [](const std::string& c) { return std::string {std::string{"\tUnknown command:"} + c + ASCII::CR_LF}; }}},

    {cmdCode::help, {help_cmd, processHelp}},
    {cmdCode::cmd1, {cmd1_cmd, processCmd1}},
    {cmdCode::cmd2, {cmd2_cmd, processCmd2}},
    {cmdCode::exit, {exit_cmd, processExit}},
    {cmdCode::quit, {quit_cmd, processQuit}},
    {cmdCode::clear, {clear_cmd, processClear}},
};
}

#endif
