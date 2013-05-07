//-------------------------------------------------------------------------------------------------
#if 0
mockConnection.hpp and mockConnection.cpp are used to supply a mock connection object for unit tests
#endif
//-------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>
#include <f8config.h>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#ifdef PROFILING_BUILD
#include <sys/gmon.h>
#endif

#include <strings.h>
#include <regex.h>
#include <errno.h>
#include <f8exception.hpp>
#include <hypersleep.hpp>
#include <mpmc.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <xml.hpp>
#include <thread.hpp>
#include <gzstream.hpp>
#include <tickval.hpp>
#include <logger.hpp>
#include <traits.hpp>
#include <timer.hpp>
#include <field.hpp>
#include <message.hpp>
#include <mockConnection.hpp>
#include <session.hpp>
#include <persist.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
int FIXReader::operator()()
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
int FIXReader::callback_processor()
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
void FIXReader::set_preamble_sz()
{
}

//-------------------------------------------------------------------------------------------------
bool FIXReader::read(f8String& to)
{
    return true;
}

//-------------------------------------------------------------------------------------------------
int FIXWriter::operator()()
{
    return 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void Connection::start()
{
    _started = true;
}

//-------------------------------------------------------------------------------------------------
void Connection::stop()
{
    _started = false;
}

