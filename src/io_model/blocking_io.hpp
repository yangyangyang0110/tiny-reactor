/* Proj: simplify_web
 * File: server.hpp
 * Created Date: 2022/7/30
 * Author: yangzilong (yangyangyang0110@outlook.com)
 * Description:
 * -----
 * Last Modified: 2022/7/30 22:15:38
 * Modified By: yang (yangyangyang0110@outlook.com)
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */
#ifndef _IO_MODEL_BLOCKING_IO_HPP_
#define _IO_MODEL_BLOCKING_IO_HPP_

#include "../helper/logger.hpp"
#include "../helper/sockets.hpp"
#include "../helper/strings.hpp"
#include "../helper/utils.hpp"
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

// Blocking IO.
class BlcokingIO {
  std::string address_;

public:
  explicit BlcokingIO(std::string address) : address_(std::move(address)) {}

  void Run() noexcept(false) {

    const auto res = ::helper::strings::splitAddress(address_);
    if (!res) {
      LOG_ERROR("Failed to parse address {}.", address_);
      exit(EXIT_FAILURE);
    }

    auto port_str = std::get<1>(res.value());
    const int port = std::stoi(std::string(port_str));
    auto socket_ = helper::sockets::initSocket(port);

    if (!socket_) {
      LOG_ERROR("Failed to create socket {}", port);
      exit(EXIT_FAILURE);
    }

    LOG_INFO("Listen {}", address_);

    int accept_fd;
    auto addr_len = sizeof(socket_.value().addr_in);
    while (true) {
      // Blocking accept a new socket.
      if ((accept_fd = accept(socket_.value().fd, (struct sockaddr *)&socket_.value().addr_in,
                              (socklen_t *)&addr_len)) < 0) {
        LOG_ERROR("Failed to Accept socket.");
        exit(EXIT_FAILURE);
      }

      LOG_INFO("Accept a new connection.");

      // Create new thread.
      std::thread([accept_fd = std::move(accept_fd)]() {
        size_t size = 0;
        char buffer[8] = {0};
#if 1
        while (true) {
          if ((size = read(accept_fd, buffer, ::helper::utils::getArraySize(buffer))) < 0) {
            break;
          }

          LOG_INFO("[{}] Receive: {}", accept_fd, buffer);
          write(accept_fd, buffer, size);
          memset(buffer, 0, ::helper::utils::getArraySize(buffer));
        }
#else
        std::stringstream ss;
        while (true) {
          while (0 < (size = read(accept_fd, buffer, ::helper::strings::getArraySize(buffer)))) {
            LOG_DEBUG("111 size is {} buffer is {}", size, buffer);
            ss << buffer;
            LOG_DEBUG("222");
            LOG_DEBUG("333");
            if (buffer[size - 1] == '\n')
              break;
            memset(buffer, 0, ::helper::strings::getArraySize(buffer));
          }
          std::string data;
          ss >> data;
          LOG_INFO("[{}] Receive: {}", accept_fd, data);
          write(accept_fd, buffer, size);
        }
#endif
        LOG_INFO("[{}] Disconntection ", accept_fd);
      }).detach();
    }
  }
};
#endif