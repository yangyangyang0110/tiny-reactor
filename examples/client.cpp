/*
 * File: client.cpp
 * Project: examples
 * File Created: 2022-09-13 1:10:23
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-13 1:13:20
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

void client(int port) {
  // create tcp && ipv4 conn.
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // fcntl(sockfd, F_SETFL, O_NONBLOCK); // set to non blocking.

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(struct sockaddr_in));
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port);
  socklen_t addr_len = sizeof(addr);

  // Convert IPv4 and IPv6 addresses from text to binary
  assert(inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) > 0);

  int client_fd;
  assert((client_fd = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr))) >= 0);

  std::string send_data = "Hi, ";
  send(sockfd, send_data.c_str(), send_data.size(), 0);
  LOG_INFO("Hello message sent");
  char recv_data[1024];
  const auto valread = read(sockfd, recv_data, helper::utils::getArraySize(recv_data));
  printf("%s\n", recv_data);
  close(client_fd);
}

int main() {
  client(Port);
  return 0;
}
