#ifndef I_SERVER_H
#define I_SERVER_H

#include <boost/asio.hpp>

#include "I_session.h"

class I_server {
 protected:
  virtual void start_accept() = 0;
  virtual void handle_accept(I_session* new_session,
                             const boost::system::error_code& error) = 0;
};

#endif