/*

Date questions become ordinary arithmetic once dates are converted to a day count. These functions
use the proleptic Gregorian calendar, which applies the modern leap rule to every year, including
those before the calendar was adopted, and they count days from the Unix epoch of 1970-01-01.

The conversion follows Howard Hinnant's closed-form algorithm, which shifts the year to start in
March so that the leap day falls at the end of the year and never interrupts the month lengths. The
remaining months then have lengths that repeat in a pattern whose running total is exactly
$\lfloor (153m + 2)/5 \rfloor$ days, and whole $400$-year eras of $146097$ days handle the leap rule
without a single division-by-$100$ special case. Both directions are branch-light formulas rather
than loops over years, so a date thousands of years away costs the same as tomorrow.

- `is_leap_year(y)` returns whether year `y` has a February 29.
- `days_in_month(y, m)` returns the length of month `m` of year `y`, where `m` is in $[1, 12]$.
- `days_from_civil(y, m, d)` returns the number of days from 1970-01-01 to the date `y`-`m`-`d`,
  which is negative for earlier dates. The date must be valid.
- `civil_from_days(days)` returns the tuple (`year`, `month`, `day`) of the date that many days from
  1970-01-01, inverting `days_from_civil()`.
- `day_of_week(y, m, d)` returns the weekday of the date, from $0$ for Sunday to $6$ for Saturday.
- `days_between(y1, m1, d1, y2, m2, d2)` returns the number of days from the first date to the
  second, which is negative when the second date is earlier.

Unix time counts seconds from the same epoch, and is exactly $86400$ times the day count plus the
seconds elapsed within the day. That decomposition is exact only because Unix time ignores leap
seconds: it is not a count of elapsed SI seconds but a label that repeats one value during a
positive leap second, which is what keeps every day $86400$ seconds long.

- `seconds_from_hms(h, m, s)` returns the seconds elapsed since midnight, and
  `hms_from_seconds(seconds)` returns the tuple (`hour`, `minute`, `second`) inverting it.
- `timestamp_from_civil(y, m, d, hh = 0, mm = 0, ss = 0)` returns the Unix timestamp of that
  instant.
- `civil_from_timestamp(t)` returns the `DateTime` of a Unix timestamp, which may be negative for
  instants before the epoch. Its day count must fit in `int`.

ISO 8601 numbers weeks from the one containing the first Thursday of the year, and assigns a date to
the year of its own week's Thursday. Early January therefore often belongs to the previous ISO year
and late December to the next, which is the mistake this replaces: 2021-01-01 is ISO week $53$ of
$2020$.

- `iso_week_date(y, m, d)` returns the tuple (`iso_year`, `week`, `weekday`) of the date, where
  `week` counts from $1$ and `weekday` runs from $1$ for Monday to $7$ for Sunday.

Easter follows the anonymous Gregorian computus, which locates the paschal full moon from the year's
place in the $19$-year Metonic cycle, corrects for the century's leap and lunar drift, then advances
to the following Sunday.

- `easter(y)` returns the (`month`, `day`) of Easter Sunday in the Gregorian year `y`, which must be
  at least $1583$.

Dates are ordinary `int` values with no range checking, and the era arithmetic stays within `int`
for any year within a few million of the epoch. The proleptic calendar disagrees with history before
a country's adoption of the Gregorian one in or after 1582, where problems usually specify Julian
rules instead: the simpler leap rule of every fourth year.

Time Complexity:
- O(1) per call to all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <tuple>
#include <utility>

struct DateTime {
  int year, month, day, hour, minute, second;
};

bool is_leap_year(int y) {
  return y % 4 == 0 && (y % 100 != 0 || y % 400 == 0);
}

int days_in_month(int y, int m) {
  assert(m >= 1 && m <= 12);
  static const int lengths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  return m == 2 && is_leap_year(y) ? 29 : lengths[m - 1];
}

int days_from_civil(int y, int m, int d) {
  assert(1 <= m && m <= 12 && 1 <= d && d <= days_in_month(y, m));
  y -= m <= 2;  // Start the year in March, so that February 29 is the final day of a year.
  int era = (y >= 0 ? y : y - 399) / 400;
  int yoe = y - era * 400;  // Year of era, in [0, 399].
  int doy =
      (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;  // Day of March-based year, in [0, 365].
  int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;     // Day of era, in [0, 146096].
  return era * 146097 + doe - 719468;                  // 719468 is the day of era of 1970-01-01.
}

std::tuple<int, int, int> civil_from_days(int days) {
  days += 719468;
  int era = (days >= 0 ? days : days - 146096) / 146097;
  int doe = days - era * 146097;
  int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int y = yoe + era * 400;
  int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  int mp = (5 * doy + 2) / 153;  // Month index of the March-based year, in [0, 11].
  int d = doy - (153 * mp + 2) / 5 + 1;
  int m = mp + (mp < 10 ? 3 : -9);
  return {y + (m <= 2), m, d};
}

int day_of_week(int y, int m, int d) {
  // 1970-01-01 was a Thursday, which is weekday 4.
  return ((days_from_civil(y, m, d) + 4) % 7 + 7) % 7;
}

int days_between(int y1, int m1, int d1, int y2, int m2, int d2) {
  return days_from_civil(y2, m2, d2) - days_from_civil(y1, m1, d1);
}

int seconds_from_hms(int h, int m, int s) {
  assert(h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60);
  return (h * 60 + m) * 60 + s;
}

std::tuple<int, int, int> hms_from_seconds(int seconds) {
  assert(seconds >= 0 && seconds < 86400);
  return {seconds / 3600, seconds / 60 % 60, seconds % 60};
}

int64_t timestamp_from_civil(int y, int m, int d, int hh = 0, int mm = 0, int ss = 0) {
  return 86400LL * days_from_civil(y, m, d) + seconds_from_hms(hh, mm, ss);
}

DateTime civil_from_timestamp(int64_t t) {
  int64_t days = t / 86400, rest = t % 86400;
  if (rest < 0) {  // Truncating division rounds toward zero, but the day count must floor.
    rest += 86400;
    days--;
  }
  assert(INT_MIN <= days && days <= INT_MAX);
  auto [y, m, d] = civil_from_days(static_cast<int>(days));
  auto [hh, mm, ss] = hms_from_seconds(static_cast<int>(rest));
  return {y, m, d, hh, mm, ss};
}

std::tuple<int, int, int> iso_week_date(int y, int m, int d) {
  int days = days_from_civil(y, m, d);
  int weekday = ((days + 3) % 7 + 7) % 7 + 1;  // The epoch was a Thursday, ISO weekday 4.
  int thursday = days - weekday + 4;           // This week's Thursday fixes the ISO year.
  int iso_year = std::get<0>(civil_from_days(thursday));
  return {iso_year, (thursday - days_from_civil(iso_year, 1, 1)) / 7 + 1, weekday};
}

std::pair<int, int> easter(int y) {
  assert(y >= 1583);
  int a = y % 19;                // Position in the Metonic cycle.
  int b = y / 100, c = y % 100;  // Century and year within the century.
  int d = b / 4, e = b % 4;      // Leap corrections for the century.
  int f = (b + 8) / 25, g = (b - f + 1) / 3;
  int h = (19 * a + b - d - g + 15) % 30;  // Days from March 21 to the paschal full moon.
  int i = c / 4, k = c % 4;
  int l = (32 + 2 * e + 2 * i - h - k) % 7;  // Days from the full moon to the next Sunday.
  int mm = (a + 11 * h + 22 * l) / 451;      // Correction for the two latest possible dates.
  return {(h + l - 7 * mm + 114) / 31, (h + l - 7 * mm + 114) % 31 + 1};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(is_leap_year(2024) && is_leap_year(2000));
  assert(!is_leap_year(2023) && !is_leap_year(1900));
  assert(days_in_month(2024, 2) == 29 && days_in_month(2023, 2) == 28);
  assert(days_in_month(2023, 12) == 31 && days_in_month(2023, 4) == 30);

  assert(days_from_civil(1970, 1, 1) == 0);
  assert(days_from_civil(1969, 12, 31) == -1);
  assert(days_from_civil(2000, 1, 1) == 10957);
  assert(civil_from_days(0) == make_tuple(1970, 1, 1));
  assert(civil_from_days(-1) == make_tuple(1969, 12, 31));
  assert(civil_from_days(10957) == make_tuple(2000, 1, 1));

  assert(day_of_week(1970, 1, 1) == 4);   // A Thursday.
  assert(day_of_week(2000, 1, 1) == 6);   // A Saturday.
  assert(day_of_week(1969, 7, 20) == 0);  // A Sunday.

  // 2024 was a leap year, so February contributed 29 days.
  assert(days_between(2024, 1, 1, 2025, 1, 1) == 366);
  assert(days_between(2023, 1, 1, 2024, 1, 1) == 365);
  assert(days_between(2024, 3, 1, 2024, 2, 1) == -29);

  assert(seconds_from_hms(23, 31, 30) == 84690);
  assert(hms_from_seconds(84690) == make_tuple(23, 31, 30));
  assert(timestamp_from_civil(1970, 1, 1) == 0);
  assert(timestamp_from_civil(2009, 2, 13, 23, 31, 30) == 1234567890);

  auto [y, mo, d, hh, mm, ss] = civil_from_timestamp(1234567890);
  assert(y == 2009 && mo == 2 && d == 13 && hh == 23 && mm == 31 && ss == 30);
  auto before = civil_from_timestamp(-1);  // One second before the epoch.
  assert(before.year == 1969 && before.month == 12 && before.day == 31);
  assert(before.hour == 23 && before.minute == 59 && before.second == 59);

  assert(iso_week_date(1970, 1, 1) == make_tuple(1970, 1, 4));
  // An ISO year runs from the Monday of the week holding the first Thursday, so it straddles
  // the calendar year on both sides.
  assert(iso_week_date(2019, 12, 30) == make_tuple(2020, 1, 1));
  assert(iso_week_date(2021, 1, 1) == make_tuple(2020, 53, 5));
  assert(iso_week_date(2026, 8, 17) == make_tuple(2026, 34, 1));

  assert(easter(2000) == make_pair(4, 23));
  assert(easter(2024) == make_pair(3, 31));
  assert(easter(2025) == make_pair(4, 20));
  auto [month, day] = easter(2026);
  assert(day_of_week(2026, month, day) == 0);  // Easter always falls on a Sunday.

  // The century rule, where 2000 is a leap year but 1900 is not.
  assert(days_between(2000, 2, 28, 2000, 3, 1) == 2);
  assert(days_between(1900, 2, 28, 1900, 3, 1) == 1);
  assert(civil_from_days(days_from_civil(2000, 2, 29)) == make_tuple(2000, 2, 29));
  assert(civil_from_days(days_from_civil(1, 1, 1)) == make_tuple(1, 1, 1));
  assert(civil_from_days(days_from_civil(-44, 3, 15)) == make_tuple(-44, 3, 15));
  return 0;
}
