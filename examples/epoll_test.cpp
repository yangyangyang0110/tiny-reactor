/*
 * File: epoll_test.cpp
 * Project: examples
 * File Created: 2022-09-13 1:23:42
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-13 1:23:44
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#include <cerrno>
#include <chrono>
#include <helper/logger.hpp>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

/**
 * @brief 注册顺序: listence -> accept -> recv -> (业务逻辑) -> send
 *
 */

static constexpr size_t MAX_EVENTS = 1024;

namespace details {

// TODO: add SFINAE or requires.
template <typename TimePrecison = std::chrono::seconds>
constexpr inline uint64_t getTimeStamp() noexcept {
  return std::chrono::duration_cast<TimePrecison>(
             std::chrono::high_resolution_clock::now().time_since_epoch())
      .count();
}

} // namespace details

struct EventData {
  using CallbackType = void(int fd, EPOLL_EVENTS etype, void *context);

  int fd;
  EPOLL_EVENTS except_etype;
  CallbackType cb;
  std::chrono::high_resolution_clock::time_point prev_action_seconds =
      std::chrono::high_resolution_clock::now();
};

class EpollWrapper {
  static int epfd_;

  static EventData eventDataSet(int fd, EPOLL_EVENTS etype, EventData::CallbackType cb) {
    EventData{
        fd,
        etype,
        cb,
    };
  }

  static void epollAdd(int fd, EPOLL_EVENTS etype) {
    struct epoll_event event;
    event.data.ptr = nullptr;
    event.events = etype;
    if (epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event) < 0) {
      LOG_ERROR("Failed to add event: {}", strerror(errno));
    }
  }

public:
  static bool InitializeListenceSocket(int port) {
    if (epfd_ != -1) {
      LOG_ERROR("Epoll fd already initialize");
      return false;
    }

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    // option: set socket non blocking.

    struct sockaddr_in addr;
    addr.sin_family = AF_INET; // IPv4 address
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // add socket to event. (linux2.6后, events大于1就可?).
    if ((epfd_ = epoll_create(MAX_EVENTS)) < 0) {
      LOG_CRITICAL("Failed to create epoll, size of events {}", MAX_EVENTS);
      exit(-1);
    }

    // create event && add it to epoll.

    // bind && listence.
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      LOG_CRITICAL("Failed to bind socket, {}", port);
      exit(-1);
    }

    if (listen(listen_fd, 20) < 0) {
      LOG_CRITICAL("Failed to listen on port {}", port);
      exit(-1);
    }
    return true;
  }
  static void AcceptCallback(){};
  static void RecvCallback(){};
  static void SendCallback(){};
};

int EpollWrapper::epfd_ = -1;

__attribute__((constructor)) static void globalInitialize() {
  helper::logger::initializeLogger(helper::logger::LogLevel::DEBUG, __FILENAME__, "", 0, 0);
}

int main() {
  globalInitialize();

  return 0;
}