//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-15 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/
//-------------------------------------------------------------------------------------------------
#if (defined(_MSC_VER) && _MSC_VER < 1700) || (!defined(_MSC_VER) && __cplusplus < 201103L)
#error Fix8 requires C++11 compiler support.
#endif
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_INCLUDES_HPP_
#define FIX8_INCLUDES_HPP_
//-------------------------------------------------------------------------------------------------
#include <fix8/f8dll.h>
#include <fix8/f8config.h>

#ifdef FIX8_PROFILING_BUILD
#include <sys/gmon.h>
#endif

#if (FIX8_REGEX_SYSTEM == FIX8_REGEX_REGEX_H)
#include <regex.h>
#elif (FIX8_REGEX_SYSTEM == FIX8_REGEX_POCO)
#include <Poco/RegularExpression.h>
#endif

#if (FIX8_THREAD_SYSTEM == FIX8_THREAD_PTHREAD)
#include <pthread.h>
#elif (FIX8_THREAD_SYSTEM == FIX8_THREAD_STDTHREAD)
#include <thread>
#else
# error Define what thread system to use
#endif

#if (FIX8_MALLOC_SYSTEM == FIX8_MALLOC_TBB)
#ifdef _MSC_VER
#include "tbb/tbbmalloc_proxy.h"
#endif
#endif

#if defined FIX8_PREPARE_MSG_SUPPORT
#include <array>
#endif
#include <unordered_map>
#include <functional>
#include <errno.h>
#include <fix8/f8exception.hpp>
#include <fix8/hypersleep.hpp>
#include <fix8/mpmc.hpp>
#include <fix8/thread.hpp>
#include <fix8/f8types.hpp>
#include <fix8/f8utils.hpp>
#include <fix8/xml.hpp>
#include <fix8/gzstream.hpp>
#include <fix8/tickval.hpp>
#include <fix8/logger.hpp>
#include <fix8/traits.hpp>
#include <fix8/timer.hpp>
#include <fix8/field.hpp>
#include <fix8/message.hpp>
#include <fix8/session.hpp>
#include <fix8/coroutine.hpp>
#include <fix8/yield.hpp>
#if F8MOCK_CONNECTION
#include "mockConnection.hpp"
#else
#include <fix8/connection.hpp>
#endif
#include <fix8/messagebus.hpp>
#include <fix8/configuration.hpp>
#include <fix8/persist.hpp>
#include <fix8/sessionwrapper.hpp>
#include <fix8/multisession.hpp>

#endif // FIX8_INCLUDES_HPP_
