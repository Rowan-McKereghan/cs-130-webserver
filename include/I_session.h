#ifndef I_SESSION_H
#define I_SESSION_H

// session interface to enable dependency injection
// both production and test sessions implement this interface
#include <boost/asio.hpp>

class I_session {
 public:
  virtual boost::asio::ip::tcp::socket& get_socket() = 0;
  virtual void Start() = 0;
};

#endif