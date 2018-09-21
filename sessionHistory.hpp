#ifndef SESSION_HISTORY_HPP
#define SESSION_HISTORY_HPP

#include "commonDefs.hpp"

namespace term_app {

class SessionHistory {
    StringList m_history {};
    typename StringList::iterator m_currCmd;

public:

    static constexpr std::size_t MAX_NUM_CMD_STORED = 5;

    SessionHistory() : m_history{1, std::string{}}, m_currCmd(m_history.begin()) { }

    SessionHistory(const SessionHistory&) = delete;
    SessionHistory& operator=(const SessionHistory&) = delete;

    void updateTopCmd(const std::string& cmd) {
        m_history.back() = cmd;
    }
    
    void completeCmd(const std::string& cmd) {
        updateTopCmd(cmd);
        if(m_history.size() >= MAX_NUM_CMD_STORED) {
            m_history.pop_front();
        }
        m_history.emplace_back(std::string{});
        m_currCmd = std::prev(m_history.end());
    }

    void clear() {
        m_history = std::list<std::string>{1, std::string{}};
        m_currCmd = m_history.begin();
    }

    std::string getPrev() {
        return (m_history.begin() == m_currCmd) ? *m_currCmd : *(--m_currCmd);
    }

    std::string getNext() {
        return (std::next(m_currCmd) == m_history.end()) ? *m_currCmd : *(++m_currCmd);
    }

    std::string getTopCmd() const {
        return m_history.back();
    }
};

}

#endif
