/* Proj: simplify_web
 * File: main.cpp
 * Created Date: 2022/7/30
 * Author: yangzilong (yangyangyang0110@outlook.com)
 * Description:
 * -----
 * Last Modified: 2022/7/30 xx:xx:xx
 * Modified By: yang (yangyangyang0110@outlook.com)
 * -----
 * Copyright (c) 2022  . All rights reserved.
 */
#include "./io_model/blocking_io.hpp"
#include "./io_model/nonblocking_io.hpp"

static constexpr char const *ADDRESS = "127.0.0.1:8080";

int main() {
  // Initialize Logger.
  helper::logger::initializeLogger(helper::logger::LogLevel::DEBUG, "SimpleEcho", "", 0, 0);

  BlcokingIO server(ADDRESS);
  server.Run();
  return 0;
}
