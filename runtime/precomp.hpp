#if !defined(FIX8_RUNTIME_PRECOMP_H_)
#define FIX8_RUNTIME_PRECOMP_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <tuple>

#ifndef _MSC_VER
#include <strings.h>
#include <sys/time.h>
#include <netdb.h>
#include <syslog.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <alloca.h>
#include <unistd.h>
#else
#include <io.h>
#define ssize_t int
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <cstdlib>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#include <cerrno>
#include <stdint.h>

#endif // FIX8_RUNTIME_PRECOMP_H_
