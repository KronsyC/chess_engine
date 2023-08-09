#pragma once
#include <stdexcept>
#include <string>
namespace chess {

class Error : std::exception {
public:
  std::string m_msg;
  Error(std::string msg) : m_msg(msg) {}

  virtual const char *what() { return m_msg.c_str(); }
};

} // namespace chess
