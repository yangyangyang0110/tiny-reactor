/*
 * File: logger.hpp
 * Project: helper
 * File Created: 2022-09-04 18:09:30
 * Author: yangyangyang (yangyangyang0110@outlook.com)
 * -----
 * Last Modified: 2022-09-04 18:10:44
 * Modified By: yangyangyang (yangyangyang0110@outlook.com>)
 * -----
 * Copyright 2022 - 2022  ,
 */

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/circular_q.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/os.h>
#include <spdlog/details/synchronous_factory.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdio>
#include <ctime>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "./define.hpp"

#define LOG_TRACE_WITH_LOGGER(logger, ...)                                                         \
  { SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__); }
#define LOG_DEBUG_WITH_LOGGER(logger, ...)                                                         \
  { SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__); }
#define LOG_WARN_WITH_LOGGER(logger, ...)                                                          \
  { SPDLOG_LOGGER_WARN(logger, __VA_ARGS__); }
#define LOG_ERROR_WITH_LOGGER(logger, ...)                                                         \
  { SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__); }
#define LOG_INFO_WITH_LOGGER(logger, ...)                                                          \
  { SPDLOG_LOGGER_INFO(logger, __VA_ARGS__); }
#define LOG_CRITICAL_WITH_LOGGER(logger, ...)                                                      \
  { SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__); }

// use fmt lib, e.g. LOG_WARN("warn log, {1}, {1}, {2}", 1, 2);
#define LOG_TRACE(msg, ...)                                                                        \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::trace, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...)                                                                        \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::debug, msg, ##__VA_ARGS__)
#define LOG_INFO(msg, ...)                                                                         \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::info, msg, ##__VA_ARGS__)
#define LOG_WARN(msg, ...)                                                                         \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::warn, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)                                                                        \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::err, msg, ##__VA_ARGS__)
#define LOG_CRITICAL(msg, ...)                                                                     \
  spdlog::log({__FILENAME__, __LINE__, __FUNCTION__}, spdlog::level::critical, msg, ##__VA_ARGS__)

namespace spdlog {
namespace sinks {

template <typename Mutex> class easy_file_sink final : public base_sink<Mutex> {
public:
  easy_file_sink(filename_t base_filename, size_t max_size, size_t max_keep_days = 0)
      : base_filename_(std::move(base_filename)), max_size_(max_size),
        max_keep_days_(max_keep_days) {
    auto now = log_clock::now();
    auto filename = gen_filename_by_daliy(base_filename_, now_tm(now));

    file_helper_.open(filename, false);
    current_size_ = file_helper_.size();
    rotation_tp_ = next_rotation_tp_();

    if (max_keep_days_ > 0) {
      filespath_q_.push_back(std::move(std::set<filename_t>()));
      filespath_q_[filespath_q_.size() - 1].insert(filename);
    }
  }

  filename_t filename() {
    std::lock_guard<Mutex> lock(base_sink<Mutex>::mutex_);
    return file_helper_.filename();
  }

protected:
  void sink_it_(const details::log_msg &msg) override {
    memory_buf_t formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    current_size_ += formatted.size();

    auto time = msg.time;
    if (time >= rotation_tp_) {
      file_helper_.close();
      auto filename = gen_filename_by_daliy(base_filename_, now_tm(time));
      file_helper_.open(filename, false);
      current_size_ = file_helper_.size();
      rotation_tp_ = next_rotation_tp_();

      {
        filespath_q_.push_back(std::move(std::set<filename_t>()));
        filespath_q_[filespath_q_.size() - 1].emplace(filename);
      }

      // Do the cleaning only at the end because it might throw on
      // failure.
      if (max_keep_days_ > 0 && filespath_q_.size() > max_keep_days_)
        delete_old_();
    } else if (current_size_ >= max_size_) {
      file_helper_.close();
      auto src_name = gen_filename_by_daliy(base_filename_, now_tm(time));
      auto target_name = gen_filename_by_filesize(base_filename_, now_tm(time),
                                                  filespath_q_[filespath_q_.size() - 1].size());

      // rename file if failed then us `target_name` as src_name.
      if (!rename_file_(src_name, target_name)) {
        details::os::sleep_for_millis(200);
        if (!rename_file_(src_name, target_name)) {
          fprintf(stderr, "%s:%d rename %s to %s failed\n", __FILENAME__, __LINE__,
                  src_name.c_str(), target_name.c_str());
          src_name = target_name;
        }
      }

      filespath_q_[filespath_q_.size() - 1].emplace(src_name);
      if (src_name != target_name)
        filespath_q_[filespath_q_.size() - 1].emplace(target_name);

      file_helper_.open(src_name, false);
      current_size_ = file_helper_.size();
      rotation_tp_ = next_rotation_tp_();
    }

    file_helper_.write(formatted);
  }

  void flush_() override { file_helper_.flush(); }

private:
  tm now_tm(log_clock::time_point tp) {
    time_t t_now = log_clock::to_time_t(tp);
    return spdlog::details::os::localtime(t_now);
  }

  /**
   * @brief Get next day tm.
   *
   * @return log_clock::time_point
   */
  log_clock::time_point next_rotation_tp_() {
    auto now = log_clock::now();
    tm date = now_tm(now);
    date.tm_hour = 0;
    date.tm_min = 0;
    date.tm_sec = 0;
    auto rotation_time = log_clock::from_time_t(std::mktime(&date));
    if (rotation_time > now)
      return rotation_time;
    return {rotation_time + std::chrono::hours(24)};
  }

  // Delete the file N rotations ago.
  // Throw spdlog_ex on failure to delete the old file.
  void delete_old_() {
    for (auto iter = filespath_q_.begin(); iter != filespath_q_.end();) {
      if (filespath_q_.size() <= max_keep_days_)
        break;

      for (const auto &it : *iter) {
        if (details::os::remove_if_exists(it) != 0)
          throw_spdlog_ex("Failed removing daily file " + details::os::filename_to_str(it), errno);
      }
      filespath_q_.erase(iter);
    }
  }

  /*  */
  static filename_t gen_filename_by_daliy(const filename_t &filename, const tm &now_tm) {
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt::format(SPDLOG_FILENAME_T("{}_{:04d}_{:02d}_{:02d}{}"), basename,
                       now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday, ext);
  }

  //
  static filename_t gen_filename_by_filesize(const filename_t &filename, const tm &now_tm,
                                             const int32_t &idx) {
    filename_t basename, ext;
    std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
    return fmt::format(SPDLOG_FILENAME_T("{}_{:04d}_{:02d}_{:02d}_{:02d}{:02d}{:02d}.{:d}{}"),
                       basename, now_tm.tm_year + 1900, now_tm.tm_mon + 1, now_tm.tm_mday,
                       now_tm.tm_hour, now_tm.tm_min, now_tm.tm_sec, idx, ext);
  }

  static bool rename_file_(const filename_t &src_filename, const filename_t &target_filename) {
    (void)details::os::remove(target_filename);
    return details::os::rename(src_filename, target_filename) == 0;
  }

  filename_t base_filename_{};
  log_clock::time_point rotation_tp_{};
  details::file_helper file_helper_{};
  std::size_t max_size_;
  std::size_t max_keep_days_;
  std::size_t current_size_{};
  std::vector<std::set<filename_t>> filespath_q_{};
};

using easy_file_sink_mt = easy_file_sink<std::mutex>;
using easy_file_sink_st = easy_file_sink<details::null_mutex>;

} // namespace sinks

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> easy_logger_mt(const std::string &logger_name,
                                              const filename_t &filename, size_t max_size,
                                              size_t max_keep_days = -1) {
  return Factory::template create<sinks::easy_file_sink_mt>(logger_name, filename, max_size,
                                                            max_keep_days);
}

template <typename Factory = spdlog::synchronous_factory>
inline std::shared_ptr<logger> easy_logger_st(const std::string &logger_name,
                                              const filename_t &filename, size_t max_size,
                                              size_t max_keep_days = -1) {
  return Factory::template create<sinks::easy_file_sink_st>(logger_name, filename, max_size,
                                                            max_keep_days);
}

} // namespace spdlog

namespace helper {
namespace logger {
enum class LogLevel {
  OFF = -1,
  DEBUG = 0,
  INFO = 1,
  WARN = 2,
  ERR = 3,
  FATAL = 4,
  TRACE = 5,
};

class LoggerGenerator {
public:
  static LoggerGenerator *get_instance() {
    static auto *logger_ptr = new LoggerGenerator;
    return logger_ptr;
  }

  static void destroy(LoggerGenerator *ptr) {
    if (ptr != nullptr) {
      delete ptr;
      ptr = nullptr;
    }
  }

  static spdlog::level::level_enum LogLevel_to_spdlog_level(const LogLevel &level) noexcept {
    spdlog::level::level_enum spd_level;
    if (LogLevel::TRACE == level)
      spd_level = spdlog::level::trace;
    else if (LogLevel::DEBUG == level)
      spd_level = spdlog::level::debug;
    else if (LogLevel::INFO == level)
      spd_level = spdlog::level::info;
    else if (LogLevel::WARN == level)
      spd_level = spdlog::level::warn;
    else if (LogLevel::ERR == level)
      spd_level = spdlog::level::err;
    else if (LogLevel::FATAL == level)
      spd_level = spdlog::level::critical;
    else if (LogLevel::OFF == level)
      spd_level = spdlog::level::off;
    return spd_level;
  }

  static std::shared_ptr<spdlog::logger> gen_logger(const spdlog::level::level_enum &level,
                                                    const std::string &logger_name,
                                                    const std::string &file_path,
                                                    size_t max_file_size, size_t max_keep_days) {
    std::vector<spdlog::sink_ptr> sinks{std::make_shared<spdlog::sinks::stdout_color_sink_mt>()};
    if (!file_path.empty() && max_file_size > 0 && max_keep_days > 0) {
      sinks.emplace_back(std::make_shared<spdlog::sinks::easy_file_sink_mt>(
          file_path, max_file_size, max_keep_days));
    }

    auto logger = std::make_shared<spdlog::logger>(logger_name, std::begin(sinks), std::end(sinks));
    logger->set_level(level);
    logger->set_pattern("[%L %Y/%m/%d %T.%e (P)%P (T)%t] [%!] [%s:%#] %v");
    return logger;
  }

  static void set_default_logger(const LogLevel &level, const std::string &logger_name,
                                 const std::string &file_name, size_t max_file_size,
                                 size_t max_keep_days) {
    spdlog::level::level_enum spd_level = LogLevel_to_spdlog_level(level);
    auto logger = gen_logger(spd_level, logger_name, file_name, max_file_size, max_keep_days);
    spdlog::set_default_logger(logger);

    // spdlog::set_level(spd_level);
    // spdlog::set_pattern("%s(%#): [%L %D %T.%e %P %t %!] %v");

    spdlog::flush_on(spd_level);
    spdlog::flush_every(std::chrono::seconds(1));
  }
};

static void initializeLogger(const LogLevel &level, const std::string &logger_name,
                             const std::string &file_path, size_t max_file_size,
                             size_t max_keep_days) {
  static LoggerGenerator loggerGenerator;
  LoggerGenerator::set_default_logger(level, logger_name, file_path, max_file_size, max_keep_days);
}

static std::shared_ptr<spdlog::logger> getSimpleLogger(const LogLevel &level,
                                                       const std::string &logger_name,
                                                       const std::string &file_path,
                                                       size_t max_file_size, size_t max_keep_days) {
  return LoggerGenerator::gen_logger(LoggerGenerator::LogLevel_to_spdlog_level(level), logger_name,
                                     file_path, max_file_size, max_keep_days);
}

} // namespace logger
} // namespace helper
