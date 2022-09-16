/*
 * File: nonblocking_io.cpp
 * Project: io_model
 * File Created: 2022-09-10 22:45:55
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-10 22:45:57
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#ifndef _IO_MODEL_NONBLOCKING_IO_HPP_
#define _IO_MODEL_NONBLOCKING_IO_HPP_

#include "../helper/logger.hpp"
#include "../helper/sockets.hpp"
#include "../helper/strings.hpp"
#include "../helper/utils.hpp"
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class NonBlocking {
  std::string address_;

  struct EventData {
    int sfd;  // socket
    int epfd; // epoll fd
    struct epoll_event events[1024];
  } event_data_;

public:
  explicit NonBlocking(std::string address) : address_(std::move(address)) {}

  void Run() const noexcept {

    // split address to get host and port.
    auto opt = ::helper::strings::splitAddress(address_);
    if (!opt) {
      LOG_ERROR("Failed to Parse host and port from {}", address_);
      return;
    }

    auto [host, port_str] = opt.value();
    const int port = stoi(std::string(port_str));

    // after linux 2.6, size param is deprecated.
    int epfd = epoll_create(1);
    int error_code = 0;

    auto socket_fd = helper::sockets::initSocket(port);

    if (!socket_fd) {
      LOG_ERROR("Failed to create socket");
    }

    struct epoll_event socket_fd_event;
    socket_fd_event.events = EPOLLIN;
    socket_fd_event.data.fd = socket_fd.value().fd;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, socket_fd.value().fd, &socket_fd_event) == -1) {
      LOG_ERROR("Error epoll_ctl");
    }

    while (true) {
      //
      epoll_event epoll_events[1024];
      // wait 1000 ms.
      const int n =
          epoll_wait(epfd, epoll_events, ::helper::utils::getArraySize(epoll_events), 1000);

      if (n == 0) {
        continue;
      } else if (n < 0) {
        // 信号中断
        if (errno == EINTR) {
          continue;
        }
        break;
      }

      for (size_t i = 0; i < n; i++) {
        if (epoll_events[i].events & EPOLLIN) {
          if (epoll_events[i].data.fd == socket_fd.value().fd) {
            // 监听socket
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int accept_fd =
                accept(socket_fd.value().fd, (struct sockaddr *)&client_addr, &client_addr_len);
            if (accept_fd < 0) {
              
            } else {
            }

          } else {
          }
        }
      }
    }

    // add socket listener.
    if (0 != (error_code = epoll_ctl(epfd, 0, 0, nullptr))) {
      LOG_ERROR("Failed to control epoll, {}", epfd);
      return;
    }

    while (true) {
      // int n_ready = epoll_wait(epfd, )
    }
  }
};
#endif
