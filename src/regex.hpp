#ifdef NO_BOOST
#include <regex>
namespace regex = std;
#else
#include <boost/regex.hpp>
namespace regex = boost;
#endif