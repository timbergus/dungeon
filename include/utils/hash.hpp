#include <cstddef>
#include <functional>
#include <utility>
struct PairHash {
  template <typename A, typename B>
  std::size_t operator()(const std::pair<A, B> &p) const {
    std::size_t h1 = std::hash<A>{}(p.first);
    std::size_t h2 = std::hash<B>{}(p.second);
    return h1 ^ (h2 << 32 | h2 >> 32);
  }
};
