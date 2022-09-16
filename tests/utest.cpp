/*
 * File: epoll_test.cpp
 * Project: tests
 * File Created: 2022-09-12 19:19:53
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-12 19:24:22
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#include <helper/logger.hpp>
#include <iostream>

__attribute__((constructor)) static void globalInitialize() {
  helper::logger::initializeLogger(helper::logger::LogLevel::DEBUG, __FILENAME__, "", 0, 0);
}

int main() {
  globalInitialize();
  LOG_INFO("Hello World");
  return 0;
}