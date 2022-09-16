/*
 * File: sockets.hpp
 * Project: helper
 * File Created: 2022-09-11 0:40:52
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-11 0:40:54
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#ifndef _HELPER_SOCKET_HPP_
#define _HELPER_SOCKET_HPP_

#include "./logger.hpp"
#include <cstring>
#include <netinet/in.h>
#include <optional>
#include <sys/epoll.h>
#include <sys/socket.h>

namespace helper::sockets {

struct Socket {
  int fd;
  struct sockaddr_in addr_in;
};

std::optional<Socket> initSocket(int port) {

  // init socket fd.
  int sockfd = 0;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    LOG_ERROR("Failed to opening socket.");
    return std::nullopt;
  }

  int opted = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opted, sizeof(opted)) < 0) {
    LOG_ERROR("Failed to set socket.");
    return std::nullopt;
  }

  // bind port.
  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET; // ipv4
  addr.sin_port = htons(port);
  if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOG_ERROR("Failed to Binding Socket Port {}.", port);
    return std::nullopt;
  }

  if (listen(sockfd, 5) < 0) {
    LOG_ERROR("Failed to Listen");
    return std::nullopt;
  }

  return Socket{sockfd, addr};
}

} // namespace helper::sockets
#endif