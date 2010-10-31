#include <config.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <bitset>

#ifdef HAS_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif

#include <strings.h>
#include <regex.h>

#include <f8exception.hpp>
#include <thread.hpp>
#include <f8types.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <message.hpp>
#include <logger.hpp>

//-------------------------------------------------------------------------------------------------
using namespace FIX8;
using namespace std;

//-------------------------------------------------------------------------------------------------
int Logger::operator()()
{
   unsigned received(0);

   for (;;)
   {
		string msg;
      _msg_queue.pop (msg); // will block
      if (msg.empty())  // means exit
         break;
		++received;

		get_stream() << msg << endl;
   }

   return 0;
}

