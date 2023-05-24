#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <cmath>
#include <limits>

class MathUtils {
public:
  static bool doubleIsEqual(double const& d1, double const& d2) {
    return std::fabs(d1 - d2) < std::numeric_limits<double>::epsilon();
  }

  static bool doubleIsSmaller(double const& d1, double const& d2) {
    return d1 + std::numeric_limits<double>::epsilon() < d2;
  }

  static bool doubleIsGreater(double const& d1, double const& d2) {
    return d1 > d2 + std::numeric_limits<double>::epsilon();
  }

  static bool doubleIsSmallerOrEqual(double const& d1, double const& d2) {
    return !doubleIsGreater(d1, d2);
  }

  static bool doubleIsGreaterOrEqual(double const& d1, double const& d2) {
    return !doubleIsSmaller(d1, d2);
  }

  static bool doubleIsMinusInfinity(double const& d1) {
    return doubleIsEqual(d1, -std::numeric_limits<double>::max());
  }

private:
  MathUtils() {}
};

#endif
