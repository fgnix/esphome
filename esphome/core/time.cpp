#include "time.h"  // NOLINT
#include "helpers.h"

#include <cinttypes>

namespace esphome {

uint8_t days_in_month(uint8_t month, uint16_t year) {
  static const uint8_t DAYS_IN_MONTH[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (month == 2 && (year % 4 == 0))
    return 29;
  return DAYS_IN_MONTH[month];
}

size_t ESPTime::strftime(char *buffer, size_t buffer_len, const char *format) {
  struct tm c_tm = this->to_c_tm();
  return ::strftime(buffer, buffer_len, format, &c_tm);
}

ESPTime ESPTime::from_c_tm(struct tm *c_tm, time_t c_time) {
  ESPTime res{};
  res.second = uint8_t(c_tm->tm_sec);
  res.minute = uint8_t(c_tm->tm_min);
  res.hour = uint8_t(c_tm->tm_hour);
  res.day_of_week = uint8_t(c_tm->tm_wday + 1);
  res.day_of_month = uint8_t(c_tm->tm_mday);
  res.day_of_year = uint16_t(c_tm->tm_yday + 1);
  res.month = uint8_t(c_tm->tm_mon + 1);
  res.year = uint16_t(c_tm->tm_year + 1900);
  res.is_dst = bool(c_tm->tm_isdst);
  res.timestamp = c_time;
  return res;
}

struct tm ESPTime::to_c_tm() {
  struct tm c_tm {};
  c_tm.tm_sec = this->second;
  c_tm.tm_min = this->minute;
  c_tm.tm_hour = this->hour;
  c_tm.tm_mday = this->day_of_month;
  c_tm.tm_mon = this->month - 1;
  c_tm.tm_year = this->year - 1900;
  c_tm.tm_wday = this->day_of_week - 1;
  c_tm.tm_yday = this->day_of_year - 1;
  c_tm.tm_isdst = this->is_dst;
  return c_tm;
}

std::string ESPTime::strftime(const char *format) {
  struct tm c_tm = this->to_c_tm();
  char buf[128];
  size_t len = ::strftime(buf, sizeof(buf), format, &c_tm);
  if (len > 0) {
    return std::string(buf, len);
  }
  return "ERROR";
}

std::string ESPTime::strftime(const std::string &format) { return this->strftime(format.c_str()); }

bool ESPTime::strptime(const std::string &time_to_parse, ESPTime &esp_time) {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  int num;

  if (sscanf(time_to_parse.c_str(), "%04hu-%02hhu-%02hhu %02hhu:%02hhu:%02hhu %n", &year, &month, &day,  // NOLINT
             &hour,                                                                                      // NOLINT
             &minute,                                                                                    // NOLINT
             &second, &num) == 6 &&                                                                      // NOLINT
      num == static_cast<int>(time_to_parse.size())) {
    esp_time.year = year;
    esp_time.month = month;
    esp_time.day_of_month = day;
    esp_time.hour = hour;
    esp_time.minute = minute;
    esp_time.second = second;
  } else if (sscanf(time_to_parse.c_str(), "%04hu-%02hhu-%02hhu %02hhu:%02hhu %n", &year, &month, &day,  // NOLINT
                    &hour,                                                                               // NOLINT
                    &minute, &num) == 5 &&                                                               // NOLINT
             num == static_cast<int>(time_to_parse.size())) {
    esp_time.year = year;
    esp_time.month = month;
    esp_time.day_of_month = day;
    esp_time.hour = hour;
    esp_time.minute = minute;
    esp_time.second = 0;
  } else if (sscanf(time_to_parse.c_str(), "%02hhu:%02hhu:%02hhu %n", &hour, &minute, &second, &num) == 3 &&  // NOLINT
             num == static_cast<int>(time_to_parse.size())) {
    esp_time.hour = hour;
    esp_time.minute = minute;
    esp_time.second = second;
  } else if (sscanf(time_to_parse.c_str(), "%02hhu:%02hhu %n", &hour, &minute, &num) == 2 &&  // NOLINT
             num == static_cast<int>(time_to_parse.size())) {
    esp_time.hour = hour;
    esp_time.minute = minute;
    esp_time.second = 0;
  } else if (sscanf(time_to_parse.c_str(), "%04hu-%02hhu-%02hhu %n", &year, &month, &day, &num) == 3 &&  // NOLINT
             num == static_cast<int>(time_to_parse.size())) {
    esp_time.year = year;
    esp_time.month = month;
    esp_time.day_of_month = day;
  } else {
    return false;
  }
  return true;
}

void ESPTime::increment_second() {
  this->timestamp++;
  if (!increment_time_value(this->second, 0, 60))
    return;

  // second roll-over, increment minute
  if (!increment_time_value(this->minute, 0, 60))
    return;

  // minute roll-over, increment hour
  if (!increment_time_value(this->hour, 0, 24))
    return;

  // hour roll-over, increment day_of_week
  increment_time_value(this->day_of_week, 1, 8);

  // hour roll-over, increment day_of_year
  const uint16_t days_in_year = (this->year % 4 == 0) ? 366 : 365;
  increment_time_value(this->day_of_year, 1, days_in_year + 1);

  // hour roll-over, increment day_of_month
  if (!increment_time_value(this->day_of_month, 1, days_in_month(this->month, this->year) + 1))
    return;

  // day of month roll-over, increment month
  if (!increment_time_value(this->month, 1, 13))
    return;

  // month roll-over, increment year
  this->year++;
}

void ESPTime::increment_minute() {
  this->timestamp += 60;

  // second roll-over, increment minute
  if (!increment_time_value(this->minute, 0, 60))
    return;

  // minute roll-over, increment hour
  if (!increment_time_value(this->hour, 0, 24))
    return;

  // hour roll-over, increment day_of_week
  increment_time_value(this->day_of_week, 1, 8);

  // hour roll-over, increment day_of_year
  const uint16_t days_in_year = (this->year % 4 == 0) ? 366 : 365;
  increment_time_value(this->day_of_year, 1, days_in_year + 1);

  // hour roll-over, increment day_of_month
  if (!increment_time_value(this->day_of_month, 1, days_in_month(this->month, this->year) + 1))
    return;

  // day of month roll-over, increment month
  if (!increment_time_value(this->month, 1, 13))
    return;

  // month roll-over, increment year
  this->year++;
}

void ESPTime::increment_hour() {
  this->timestamp += 3600;

  // minute roll-over, increment hour
  if (!increment_time_value(this->hour, 0, 24))
    return;

  // hour roll-over, increment day_of_week
  increment_time_value(this->day_of_week, 1, 8);

  // hour roll-over, increment day_of_year
  const uint16_t days_in_year = (this->year % 4 == 0) ? 366 : 365;
  increment_time_value(this->day_of_year, 1, days_in_year + 1);

  // hour roll-over, increment day_of_month
  if (!increment_time_value(this->day_of_month, 1, days_in_month(this->month, this->year) + 1))
    return;

  // day of month roll-over, increment month
  if (!increment_time_value(this->month, 1, 13))
    return;

  // month roll-over, increment year
  this->year++;
}

void ESPTime::increment_day() {
  this->timestamp += 86400;

  // increment day_of_week
  increment_time_value(this->day_of_week, 1, 8);

  // increment day_of_year
  const uint16_t days_in_year = (this->year % 4 == 0) ? 366 : 365;
  increment_time_value(this->day_of_year, 1, days_in_year + 1);

  // increment day_of_month
  if (!increment_time_value(this->day_of_month, 1, days_in_month(this->month, this->year) + 1))
    return;

  // day of month roll-over, increment month
  if (!increment_time_value(this->month, 1, 13))
    return;

  // month roll-over, increment year
  this->year++;
}

void ESPTime::increment_week() {
  this->timestamp += 86400 * 7;

  // increment day_of_year
  const uint16_t days_in_current_year = (this->year % 4 == 0) ? 366 : 365;
  this->day_of_year += 7;
  if (this->day_of_year > days_in_current_year)
    this->day_of_year -= days_in_current_year;

  // increment day_of_month
  const uint8_t days_in_current_month = days_in_month(this->month, this->year);
  this->day_of_month += 7;
  if (this->day_of_month <= days_in_current_month)
    return;
  this->day_of_month -= days_in_current_month;

  // day of month roll-over, increment month
  if (!increment_time_value(this->month, 1, 13))
    return;

  // month roll-over, increment year
  this->year++;
}

void ESPTime::increment_second_local() {
  this->increment_second();

  // https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
  // DST occurs only at minute 00 and second 00
  if (this->second != 0 || this->minute != 0)
    return;

  // DST happens, depending on the country, between 00 and 04 o'clock
  if (this->hour > 4)
    return;

  // DST happens only on Friday, Saturday and Sundays
  if (this->day_of_week > 1 && this->day_of_week <= 5)
    return;

  // Recompute the exact time from the timestamp, if a DST change MAY have occurred
  *this = ESPTime::from_epoch_local(this->timestamp);
}

void ESPTime::increment_minute_local() {
  this->increment_minute();

  // https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
  // DST occurs only at minute 00 and second 00. But since we incremented by 60 seconds for sure we did went
  // through the second 0
  if (this->minute != 0)
    return;

  // DST happens, depending on the country, between 00 and 04 o'clock
  if (this->hour > 4)
    return;

  // DST happens only on Friday, Saturday and Sundays
  if (this->day_of_week > 1 && this->day_of_week <= 5)
    return;

  // Recompute the exact time from the timestamp, if a DST change MAY have occurred
  *this = ESPTime::from_epoch_local(this->timestamp);
}

void ESPTime::increment_hour_local() {
  this->increment_hour();

  // https://en.wikipedia.org/wiki/Daylight_saving_time_by_country
  // DST occurs only at minute 00 and second 00. But since we incremented by 3600 seconds for sure we did went
  // through the second 0 and the minute 0

  // DST happens, depending on the country, between 00 and 04 o'clock
  if (this->hour > 4)
    return;

  // DST happens only on Friday, Saturday and Sundays
  if (this->day_of_week > 1 && this->day_of_week <= 5)
    return;

  // Recompute the exact time from the timestamp, if a DST change MAY have occurred
  *this = ESPTime::from_epoch_local(this->timestamp);
}

void ESPTime::increment_day_local() {
	const time_t old_timestamp = this->timestamp;
	const time_t no_dst_changes_delta = 86400; // 1 day in seconds
	this->increment_day();
	this->recalc_timestamp_local();

	// DST ended. The day when the DST ends has 1 extra hour thus the timestamp is greater
	if (this->timestamp > old_timestamp + no_dst_changes_delta) {
		this->is_dst = false;

	// DST started. The day when the DST starts has 1 hour less, thus the timestamp is lower
	} else if (this->timestamp < old_timestamp + no_dst_changes_delta) {
		this->is_dst = true;
	}
}

void ESPTime::increment_week_local() {
	const time_t old_timestamp = this->timestamp;
	const time_t no_dst_changes_delta = 86400 * 7; // 7 day in seconds
	this->increment_week();
	this->recalc_timestamp_local();

	// DST ended. The day when the DST ends has 1 extra hour thus the timestamp is greater
	if (this->timestamp > old_timestamp + no_dst_changes_delta) {
		this->is_dst = false;

	// DST started. The day when the DST starts has 1 hour less, thus the timestamp is lower
	} else if (this->timestamp < old_timestamp + no_dst_changes_delta) {
		this->is_dst = true;
	}
}

void ESPTime::recalc_timestamp_utc(bool use_day_of_year) {
  time_t res = 0;
  if (!this->fields_in_range()) {
    this->timestamp = -1;
    return;
  }

  for (int i = 1970; i < this->year; i++)
    res += (i % 4 == 0) ? 366 : 365;

  if (use_day_of_year) {
    res += this->day_of_year - 1;
  } else {
    for (int i = 1; i < this->month; i++)
      res += days_in_month(i, this->year);
    res += this->day_of_month - 1;
  }

  res *= 24;
  res += this->hour;
  res *= 60;
  res += this->minute;
  res *= 60;
  res += this->second;
  this->timestamp = res;
}

void ESPTime::recalc_timestamp_local() {
  struct tm tm;

  tm.tm_year = this->year - 1900;
  tm.tm_mon = this->month - 1;
  tm.tm_mday = this->day_of_month;
  tm.tm_hour = this->hour;
  tm.tm_min = this->minute;
  tm.tm_sec = this->second;
  tm.tm_isdst = -1;

  this->timestamp = mktime(&tm);
}

void ESPTime::recalc_day_of_year() {
  uint16_t res = 0;
  for (int i = 1; i < this->month; i++)
    res += days_in_month(i, this->year);
  this->day_of_year = res + this->day_of_month;
}

int32_t ESPTime::timezone_offset() {
  time_t now = ::time(nullptr);
  struct tm local_tm = *::localtime(&now);
  local_tm.tm_isdst = 0;  // Cause mktime to ignore daylight saving time because we want to include it in the offset.
  time_t local_time = mktime(&local_tm);
  struct tm utc_tm = *::gmtime(&now);
  time_t utc_time = mktime(&utc_tm);
  return static_cast<int32_t>(local_time - utc_time);
}

bool ESPTime::operator<(const ESPTime &other) const { return this->timestamp < other.timestamp; }
bool ESPTime::operator<=(const ESPTime &other) const { return this->timestamp <= other.timestamp; }
bool ESPTime::operator==(const ESPTime &other) const { return this->timestamp == other.timestamp; }
bool ESPTime::operator>=(const ESPTime &other) const { return this->timestamp >= other.timestamp; }
bool ESPTime::operator>(const ESPTime &other) const { return this->timestamp > other.timestamp; }

template<typename T> bool increment_time_value(T &current, uint16_t begin, uint16_t end) {
  current++;
  if (current >= end) {
    current = begin;
    return true;
  }
  return false;
}

}  // namespace esphome
