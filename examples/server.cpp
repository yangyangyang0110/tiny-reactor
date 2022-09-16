/*
 * File: server.cpp
 * Project: examples
 * File Created: 2022-09-13 1:10:20
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-13 1:10:31
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#include "./global_vars.hpp"
#include <arpa/inet.h>
#include <cassert>
#include <chrono>
#include <fcntl.h>
#include <helper/logger.hpp>
#include <helper/strings.hpp>
#include <helper/utils.hpp>
#include <iostream>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>


void server(int port) {
  // create tcp && ipv4 conn.
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // fcntl(sockfd, F_SETFL, O_NONBLOCK); // set to non blocking.

  int opted = 1;
  assert(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opted, sizeof(opted)) >= 0);

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  socklen_t addr_len = sizeof(addr);
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_family = AF_INET; // ipv4
  // 需要开放到网络中嘛?
  addr.sin_port = htons(port); // convert to byte order(big endian).

  assert(bind(sockfd, (struct sockaddr *)&addr, addr_len) >= 0);

  // backlog: queue size for number socket.
  assert(listen(sockfd, 5) >= 0);

  // waiting a accept.
  int accept_fd = accept(sockfd, (struct sockaddr *)&addr, &addr_len);

  // only get 1024 bytes.
  char buffer[1024];
  const int len = read(accept_fd, buffer, ::helper::utils::getArraySize(buffer));
  LOG_INFO("Receive from [:] {}", buffer);
  write(accept_fd, buffer, len);

  // close && shutdown.
  close(accept_fd);
  shutdown(accept_fd, SHUT_RDWR);
}

int main() {
  server(Port);
  return 0;
}
