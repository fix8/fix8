# Changelog

## Table of Contents
- [[2.0.1](#201)] - 2025-06-02
- [[1.4.3](#143)] - 2023-05-10
- [[1.4.1](#141)] - 2019-01-01
- [[1.4.0](#140)] - 2016-09-16
- [[1.3.4](#134)] - 2015-08-23
- [[1.3.3](#133)] - 2015-04-24
- [[1.3.2](#132)] - 2015-01-02
- [[1.3.1](#131)] - 2014-08-24
- [[1.3.0](#130)] - 2014-08-17
- [[1.2.0](#120)] - 2014-06-04
- [[1.1.0](#110)] - 2014-04-06
- [[1.0.0](#100)] - 2014-01-16
- [[0.10.0](#0100)] - 2013-11-10
- [[0.9.6](#096)] - 2013-10-13
- [[0.9.5](#095)] - 2013-09-22
- [[0.9.4](#094)] - 2013-08-25
- [[0.9.3](#093)] - 2013-08-04
- [[0.9.2](#092)] - 2013-07-21
- [[0.9.0](#090)] - 2013-06-29
- [[0.8.0](#080)] - 2013-05-10
- [[0.7.2](#072)] - 2013-04-07
- [[0.7.0](#070)] - 2013-02-23
- [[0.6.7](#067)] - 2013-01-28
- [[0.6.6](#066)] - 2012-12-16
- [[0.6.5](#065)] - 2012-12-09
- [[0.6.4](#064)] - 2012-11-22
- [[0.6.3](#063)] - 2012-11-18
- [[0.6.2](#062)] - 2012-10-23
- [[0.6.1](#061)] - 2012-09-07
- [[0.6.0](#060)] - 2012-07-21
- [[0.5.7](#057)] - 2012-05-25
- [[0.5.5](#055)] - 2012-05-12
- [[0.5.1](#051)] - 2012-04-29
- [[0.5.0](#050)] - 2012-04-22
- [[0.4.17](#0417)] - 2012-03-20
- [[0.4.16](#0416)] - 2012-03-08
- [[0.4.15](#0415)] - 2012-03-01
- [[0.4.12](#0412)] - 2012-02-19
- [[0.4.10](#0410)] - 2012-02-05
- [[0.4.4](#044)] - 2012-01-27
- [[0.4.2](#042)] - 2012-01-23
- [[0.3.58](#0358)] - 2012-01-07
- [[0.1.2](#012)] - 2010-09-11
---

<a id="201"></a>
## [2.0.1] - 2025-06-02
*Maintainer: David L. Dight <fix@fix8.org>*
- cmake build system replaces GNU autotools
- Minimum C++17 required
- Fixed multiple runtime library compiler warnings
- Removed tcmalloc, replaced with tbbmalloc
- Removed FastFlow, replaced with tbb
- Removed extern "C" linkage warnings
- cmake downloads and builds all default required deps
- Fixed `session_test` errors
- Fixed build for MacOS
- Fixed build for Windows, builds in MSVC, VSCode
- Added zlib, getopt for Windows
- Shared and static runtime libs built by default
- Static lib contains all external deps
- Added Intel compiler support
- Upgrade poco, removed deprecated calls
- Updated and merged bug fixes
- Updated compiler to support later FIX EPs
- Includes FIX 5.0 SP2 and FIXT 1.1 Extension Packs 264 in Quickfix schema format
- Removed cruft
<a id="143"></a>
## [1.4.3] - 2023-05-10
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed build issues hftest, utests, some compiler warnings
- Updated checksum algorithm
- Merged from dev 1.4.2
<a id="141"></a>
## [1.4.1] - 2019-01-01
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-729,FX-744,FX-821,FX-884
- Fixed deadlock condition in heartbeat
- Fixed memory leaks
- Fixed f8c incorrectly removing unused fields from the top of fields list
- Fixed steroid chksum algorithm giving incorrect values for large messages
<a id="140"></a>
## [1.4.0] - 2016-09-16
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-530,FX-533,FX-539,FX-562,FX-563,FX-564,FX-560,FX-588,FX-569,FX-596,FX-642,FX-633,FX-615,FX-609,FX-655
- Added verison tag to Nuget packages
- Fixed Seqedit reports corrupted persister index
- Added Provide programmatic way to set reset sequence number flag on logon
- Fixed XML parser should report line numbers of mismatched element start/end
- Added Compiler should optionally report unused tags
- Added Session should provide callback for rejected inbound message
- Fixed XmlData fields unsupported
- Fixed Rejected inbound messages do not appear in protocol log
- Fixed FIX time to epoch converter
- Fixed Acceptor mode: Crash while receiving logout message
- Fixed Sequence number reset does not function correctly
- Fixed Expected Sequence number reaches extreme and unrealistic value
- Upgraded FastFlow to v2.1.0
- Added ConsoleMenu permit messages to be created from inbound messages
- Added ConsoleMenu SelectMsgFrom now displays message sending time and seqnum if available
- Fixed crash on "Send one message, optionally save before send"
- Fixed f8c unhandled exception while stoul'ing fields
- Fixed `login_retries="0"` not working
- Fixed Expected Sequence number reaches extreme and unrealistic value
- Fixed Crashes on heartbeat
<a id="134"></a>
## [1.3.4] - 2015-08-23
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-508, FX-511, FX-490, FX-491, FX-480, FX-470, FX-336, FX-525, FX-524, FX-523, FX-520, FX-516, FX-527
- Client logout crashes FIX server
- SessionWrapper needs to be cleaned up in the destructor
- Fixed Seqedit reports corrupted persister index
- Fixed Provide hook in Session to modify header before sending
- Fixed ReliableClientSession crash when connection failed
- Fixed Crash on sending cloned message
- Added Provide optional improved checksum calculation
- Added Provide Consolemenu method to remove msg from list and return to application
- Fixed Replace `get_value<>` with `stoi`, `stoul`, `stof`, etc
- Added Provide non-const header and trailer accessors
- Fixed With f8config installed in system includes, #defines causes namespace pollution
- Fixed Test harness improvements, testing
<a id="133"></a>
## [1.3.3] - 2015-04-24
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-321, FX-319, FX-336, FX-480
- Fixed Provide capability to build stock FIX libraries
- Fixed seqedit Poco linkage prolem
- Added CMake find_package support
- Fixed `ssout_xxxx()` macros can be used outside FIX8 namespace
- Fixed `Schedule::test` bug fix for calculating "today" in local time zone
- Fixed support application framework to manage all permutations of process_model and mode
- Fixed issue in MessageBase::clear
- Fixed XML parser does not support CDATA values
- Fixed ReliableClientSession crash when connection failed
- Fixed Client logout crashes FIX server
<a id="132"></a>
## [1.3.2] - 2015-01-02
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-394, FX-385, FX-379, FX-372, FX-371, FX-355, FX-353, FX-350, FX-321,FX-328, FX-326, FX-307, FX-332, FX-333, FX-369, FX-354, FX-323
- Fixed f8test client and server core dumps on exit when compiled with stdthread
- Fixed `copy_legal` causes segfault on windows
- Fixed schedule `is_valid()` returns true even schedule is invalid under win
- Fixed Fix8 has empty timestamps under windows
- Fixed client session reconnect failure after previous abnormal session disconnect
- Fixed unhandled message and reject problems
- Fixed invalid tag in test log of Win build
- Fixed `Tickval::todouble` returning 0
- Fixed provide capability to build stock FIX libraries
- Fixed improve VS2013 build wth stock FIX schemas
- Fixed replace time/date handling (Tickval) with C++11 std::chrono
- Fixed ReliableClientSession crash when socket connection refused
- Fixed Fix8 test harness (client) improvements
- Fixed provide support for longname field lookup
- Fixed add git revision & fix8 version reporting to log during fix8 start
- Fixed crashing in hftest of Win build
- Fixed on exit when using std::thread, logger reports "resource deadlock avoided"
<a id="131"></a>
## [1.3.1] - 2014-08-24
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-325
- 1.3 build bug under Win
<a id="130"></a>
## [1.3.0] - 2014-08-17
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-309, FX-308, FX-307, FX-306, FX-305, FX-304, FX-303, FX-302, FX-301,FX-300, FX-299, FX-298, FX-296, FX-295, FX-294, FX-293, FX-292, FX-291, FX-290, FX-289, FX-288, FX-287, FX-286, FX-236, FX-310, FX-312, FX-311 FX-297, FX-281, FX-233, FX-231, FX-313, FX-315, FX-316, FX-317, FX-318
- Provide XML logger
- Overhaul logging system
- Loggers should support log level
- Allow shallow message construction
- Provide `Message::move_legal`
- XML parser does not support CDATA values
- gcc 4.7.2 linux build broken
- Allow shallow message construction
- Provide `Message::move_legal`
- Facilitate "pass-through" fields which are not mentioned in the Dictionary
- MAGIC_NUM expression can cause problems
- Support package and configuration string queries
- Provide flag settings to control XML parser
- Remove main Nuget package link dependency to gtest
- Replace `FIX8::dthread` with `std::thread`
- ReliableClientSession crash when socket connection refused
- Compilation error on clean checkout 14/07/10
- Protocol Logger Thread is not destroyed after deleting session
- Wrong Timer implementation
- RAII std::ostream Singleton log target
- `globalLogger::create_instance` needs refactoring
- Precision was altered unintentionally
- Support flattened field query in messages
- Allow SingleLogger to accept user defined LogFlags
- Add mini-timestamp flag to logger
- hftest server exits when sending preloaded messages under windows
- Update on 14-06-24 introduced build warnings on win
- Fix variadic templates compile error under VS2013
- Windows build fails when configured w/o TBB
- Create VS2012 build of FIX8
- Provide programmatic/generic method of loading and using Fix8 metadata
- Permit lookup of fields and messages by their long name
- Xml improvements: `find_child`, `GetLocString`
- Field equivalence operators missing
- Option to compiler to generate router stubs without defaults
- Session state does not changed when connection goes down.
- Allow the option to the getters from a fix message those values that are fixed point values to be stored in float instead of double
- Make dist, rpmbuild and pro build broken
- Make nuget package generation files (*.autopkg) to be version independent
<a id="120"></a>
## [1.2.0] - 2014-06-04
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-278, FX-276, FX-275, FX-274, FX-273, FX-272, FX-271, FX-270, FX-269,FX-268, FX-267, FX-266, FX-265, FX-264, FX-263, FX-228, FX-261, FX-260, FX-259, FX-258, FX-257, FX-256, FX-253, FX-252, FX-251, FX-250, FX-249, FX-248, FX-247, FX-246, FX-245, FX-244, FX-243, FX-242, FX-241, FX-240, FX-239, FX-238, FX-237, FX-236, FX-235, FX-233, FX-232, FX-231, FX-230, FX-229, FX-282, FX-280, FX-279, FX-283, FX-285, FX-176, FX-220, FX-195, FX-217
- Provide access to raw inbound and outbound FIX message text
- Make --enable-extended-metadata work in windows
- Provide a session configuration option to enable or disable retransmission
- Provide tabsize setting to customise fix printer
- Update wiki with instruction of building NuGet packages
- On Mac OS X Maverick, clang generates lots of warnings
- building with `--enable-tbb-malloc=yes` on OS X gives error
- poco error under OS X Maverics
- Realm range not working as expected
- Poco On Windows
- Add a few helper methods to BaseEntry and BaseMsgEntry
- Mandatory fields not propagating through compiler with FIXT
- MarketDataRequest with certain fields throws exception invalid field
- `default_appl_ver_id` (1137) applied if configured, regardless of FIX version
- Example of how to subscribe to MarketData
- Distinguish between invalid and unknown field exceptions in message
- Replace StaticTable with std::map
- Facilitate Fix8Pro and open source common build
- Segfault in ~Session::Session/Session:stop
- Create FIX8 NuGet package
- Create a .net port of fix8
- Upgrade bundled FastFlow from 2.0.2 to 2.0.4
- Replace `FIX8::f8_atomic` with `std::atomic`
- FX-242 Write Fix8 1.1 to 1.2 migration guide
- Replace all the sizes from unsigned to size_t
- OSX g++-4.2.1 on mac does not support `-fno-var-tracking-assignments`
- Invalid inbound acceptor SenderCompID ignored
- Create OSX HOWTO in Confluence
- Acceptor SenderCompID not configurable
- Provide test example for multi-session support
- Provide support for underlying FieldType introspection
- Support optional CompID enforcement
- Replace `FIX8::scoped_ptr` with `std::unique_ptr`
- Client logs should be created with SessionID suffix
- Restructure Session wrapper classes to support non-templated base classes
- Allow user to set `SO_KEEPALIVE` option from config
- Complete confluence documentation for 1.1.0 and 1.2.0 features
- Provide support and management for multiple ServerInstances
- Server support for predefined set of remote SenderCompIDs
- Allow user to set `SO_REUSEADDR` option from config
- Allow user to set `SO_LINGER` option from config
- Support defaults section in Session xml configuration
- FX-41 Replace `FIX8::dthread` with `std::thread`
- Add option to `FIX8::logger` to suppress LF on logline
- Windows build fails when configured w/o TBB
- Linux End-of-Line Charactor not handled by Message::factory
- hftest server exits when sending preloaded messages under windows
- f8print will not decode f8test runs properly
- Cmd line scripts do not like spaces in path when building fix8 under Windows
- Move compilation to use precompiled header
<a id="110"></a>
## [1.1.0] - 2014-04-06
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-223, FX-222, FX-221, FX-219, FX-216, FX-214, FX-213, FX-212, FX-211, FX-209,FX-206, FX-205, FX-204, FX-200, FX-198, FX-193, FX-192, FX-191, FX-189, FX-187, FX-185, FX-183, FX-180, FX-178, FX-177, FX-175, FX-184, FX-224, FX-225, FX-226, FX-227, FX-203, FX-230
- Fixed Win64 build failed with seqnum mismatch
- Fixed f8c compiler crashes if schema file not found
- Fixed Logfile creation should handle new paths
- Added session state change event callback
- Fixed Won't reconnect if exchange log session out.
- Fixed Deadlock in retransmission behaviour
- Added global logger needs optional file and line attributes when logging
- Added provide redis persister
- Fixed Session silently ignores no logger, no plogger and no persister errors
- Fixed Speedup Win build
- Update Windows Wiki (confluence) page
- Fixed Make auto linking fix8 lib optional
- Fixed Remove public static vars from generated code
- Fixed hftest works incompletely under win32/64
- Added malloc configuration defines to f8config.h
- Fixed Test fix8 with onload/SFC 10G cards
- Fixed Reliable session fails to re-connect on connection errors
- Fixed UTs build fails when running via make
- Add support for Session based BusinessMessageReject ('j')
- Added support for Session login and logout time
- Added make socket read buffering optional
- Added expose `FIX8::Session` scheduler to user session class
- Added support for Session Start time and End Time
- Fixed Hang in FIXReader::sockRead
- Fixed gcc 4.2 and greater supports `-fno-var-tracking -fno-var-tracking-assignments`
- Fixed Complete build options for Pthread API
- Added message handling: allow non-const operations
- Fixed Non-standard XML attribute comment problem
- Added mechanism to support ad-hoc message recycling
- Added permit alternate source/header extensions when generating code
- Added generic access to key/value pair put() and get() in Persister
- Added precompiled header to f8c generated files
- Fixed f8print will not decode f8test runs properly
<a id="100"></a>
## [1.0.0] - 2014-01-16
*Maintainer: David L. Dight <fix@fix8.org>*
- Our first official GA release
- Fixed Jira tickets FX-107, FX-129, FX-130, FX-131, FX-134, FX-139, FX-140, FX-141, FX-142, FX-143, FX-144, FX-146, FX-147, FX-149, FX-150, FX-151, FX-157, FX-158, FX-159, FX-161, FX-162, FX-163, FX-164, FX-165, FX-166, FX-167, FX-168, FX-169, FX-170, FX-173, FX-174
- Provide support for basic client failover capability
- Make connection timeout configurable
- Add support for non-standard XML attribute comments
- Add support for OSX
- Add repeating group test cases for permissive mode
- Support a permissive message field mode in decoder
- Provide override to -fno-var-tracking-assignments
- Fixed `Session::send_process()` dumps core in reliable mode if client drops connection before login
- Added When working in coro mode there has to be a flag that session is ready to operate
- Added When working in coro mode reader.execute() calls to base class operator()
- Merged fix8:dev with fix8:master
- Added MonthYear and LocalMktDate Date Formats
- Fixed f8test doesn't work as expected
- Fixed clang 3.2-3.4 compilation warnings
- Fixed Sending of FIX message takes too long
- Added Batch message sending
- Fixed `MessageBase::extract_element(..., f8String& tag, f8String& val)` is ineffective
- Fixed FIXReader calls sockRead too many times
- Added Make includes relative to project root
- Added SSL support
- Fixed `XmlElement::find` with attribute and value not finding correctly
- Fixed Segfault with non-set SessionConfig on heartbeat
- Added Support custom field addition on f8c command line
- Fixed Fields of type 'data' are not parsed according to FIX specification
- Fixed `time_to_epoch` tm_mon ternary operator does not allow January dates to be converted to an epoch timestamp
- Fixed Makefile.am does not reference f8dll.h
- Fixed Build error on OS X -rdynamic
- Fixed error: `'uint32_t'` does not name a type
- Fixed `CLOCK_REALTIME` error on compilation on Windows
- Fixed Make include guards standard-compliant
- Code freeze for GA 1.0.0 final
<a id="0100"></a>
## [0.10.0] - 2013-11-10
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-102, FX-103, FX-104, FX-105, FX-107, FX-110, FX-113, FX-114, FX-115, FX-116, FX-119, FX-120, FX-122, FX-124, FX-127,FX-122, FX-124, FX-127
- Fixed allow sender to take ownership of messages after send
- Fixed removed message recycling
- Fixed session::send not thread safe with multiple senders in threaded mode
- Fixed TBB allocator is not used when linking to tbb
- Fixed there is no itoa for int64
- Fixed hb interval is set 1 when using reliable connection
- Fixed multiple instances of FIX8 session share the last messages table
- Fixed groups with 0 elements are not processed
- Fixed error when sending message with BodyLength > 9999
- Fixed issues with Windows build
- Added Fix8 include path in generated files are now configurable
- Added -P switch to f8c to embed fix8 in include paths
- Added order batch send mode
- Added allow application to detach messages when received from framework: `Session::handle_application` API change
- Added provide way to set default precison for floating point values
<a id="096"></a>
## [0.9.6] - 2013-10-13
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-76, FX-93, FX-94, FX-95, FX-96, FX-97, FX-98, FX-99, FX-100, FX-101
- Decode latency reduced; throughput now 3x quickfix
- Fixed SIOF - static initialisation inconsistent on different platforms; use ctx() instead of ctx
- Fixed compiler treats all repeating groups with the same name as common
- Fixed core dump on message or field instantiation
- Resolved not build tests on centos 6.4
- Fixed required Fields in Optional Components Should be Flagged as Optional
- Fixed remove FieldTraits reserve behaviour
- Added provide switch to suppress doxygen warnings
- Added missing some date/time related fields
- Fixed Fix8 does not build properly on arm. Test cases build and pass on ARMv7
- Fixed link dependencies for clang compilation
- Added -C switch to f8c to turn off version checking
- Added -I switch to f8c, providing more info about build config and platform
- Added -W switch to f8c, to suppress warning messages
<a id="095"></a>
## [0.9.5] - 2013-09-22
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-78, FX-79, FX-80, FX-81, FX-82, FX-83, FX-84, FX-85, FX-86, FX-87, FX-88, FX-89, FX-90, FX-91, FX-29, FX-92, FX-89, FX-90, FX-91, FX-29, FX-92
- Fixed SendingTime and TransactTime not being output by Fix8 printer
- Added compiler option to suppress realm use during field construction
- Replaced `Poco::DateTime` with custom date time parser, reduced decode latency ~ 20%
- Fixed f8c compiler crashing on exit
- Fixed Incorrect sequence number in GenerateSequenceReset
- Removed coroutine process mode spinlocks
- Workaround for f8test not building on low memory platforms or with older compilers
- Skip formal decode of some header/trailer fields
- Fixed error checking on logfile creation
- Fixed `F8MetaCntx::_bme.find()` not returning end() if not found
- Fixed ignore_logon_sequence_check check core dumping in client
- Added permit applications to by-pass chksum checking
- Replace field string parameter with const char *
- Fixed generated files should not build with newer framework versions
- Fixed replace compiler f8c generated instantiators with compiler generated versions
- Fixed bug with some linux distros, threaded sessions core dump on exit
- Replaced #ifdef 0 comment blocks with /* ... */
- Templated `Field::add_field`
- Improved fix printer formatting, removed incorrect group metadata
- Added rdtsc option for codec timing
- Added `set_scheduler` and `set_affinity` support
<a id="094"></a>
## [0.9.4] - 2013-08-25
*Maintainer: David L. Dight <fix@fix8.org>*
- Merged in Richard Bourne's Windows port.
- Fixed Jira tickets FX-72, FX-73, FX-74, FX-75
- Merged from evdubs: remove the friend declaration in `f8_scoped_lock_impl`
- Fixup package spec for pre-release to Fedora (now builds on f20 rawhide)
- Added `ReliableClientSession::has_given_up()`
- Fixed: Gcc locks up with compiler generated traits file; reduced _traits.cpp file by 40%
- Fixed: Compile error with gcc 4.8.1
- Fixed: XML character entity parsing broken; extended entity set;
- Fixed: XML parser does not provide meaningful indication of errors.
<a id="093"></a>
## [0.9.3] - 2013-08-04
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-67, FX-68, FX-69, FX-70, FX-71
- Fixed race condition in Singleton
- Added backup the persist file instead of purging after sequence reset
- Added forced logout message should contain error text
- Fixed server crashes when reliable client attempts sync reconnect
- Fixed reset sequence number not truncating BDB persist database
<a id="092"></a>
## [0.9.2] - 2013-07-21
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-64, FX-65, FX-66
- Merged Neomantra changes allows Fix8 to be built using C++11 compiler
- Fixed sequence reset persist database not purged
- Fixed reset sequence number on logon not working
- Fixed persister not writing any data
<a id="090"></a>
## [0.9.0] - 2013-06-29
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-57, FX-58, FX-59, FX-60, FX-61, FX-62, FX-63
- Logflags - support specific inbound and outbound flags for protocol logs
- Replace cfpopen with non GPL replacement
- Fixed compiler generated include guards do not work if alternate output directory specified.
- Fixed f8c compiler - can't specify output directory by "-o" as expected
- Remove header/trailer field lookups in encode and decode
- Fixed unit tests not building properly
- Add optional coroutine version of FIX reader and writer
<a id="080"></a>
## [0.8.0] - 2013-05-10
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed Jira tickets FX-56, FX-55, FX-54, FX-53, FX-52, FX-51, FX-50, FX-49, FX-48, FX-47, FX-46, FX-45, FX-31
- Integrate a 3rd party unit testing framework
- Integrate gperf tcmalloc alternate heap allocator
- Modify session to force sequence number assignment when requested
- Improve fmt_chksum routine
- Implement hash array index lookup for fields
- Provide component metadata visibility
- Replace double field encode (sprintf) with modp_dtoa
- Remove custom field support
- Remove some virtual methods from fields and generated messages
- Fix Fastflow install not placing includes in correct path
- Fix MemoryPersister::get not handling end record situation correctly
- Fix Invalid Session::handle_resend_request not resetting session state
- Fix In get_last_seqnum(unsigned& to) of MemoryPersister, can't get last seqnum from the argument "to"
- Fix hftest preload should preload on startup
<a id="072"></a>
## [0.7.2] - 2013-04-07
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira tickets FX-23, FX-35, FX-36, FX-37, FX-38, FX-39, FX-40, FX-42, FX-43, FX-44, FX-17
- Partial implemntation of stack based messages.
- TBB optional; Fastflow now used; pipelining options; codec timings improvements;
- Added man-pages for seqedit and f8c
- Fixed under load, server disconnects client reporting it has timed out on receiving messages
- Provide utility to edit next send/expected receive in persistence files
- Investigate and perhaps deploy fastflow lock free containers to replace TBB
- Permit selection of pipelined or non-pipelined operation, through session config
- Change performance test application to provide better measure of codec performance
- Make Intel TBB optional
- Fixed `FileLogger::rotate()` not working as expected
- Use FastFlow queue processing
- Fixed FIXReader, FIXWriter dropping bytes when buffers full
- Fixed build hftest issue
<a id="070"></a>
## [0.7.0] - 2013-02-23
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira tickets FX-34, FX-33, FX-32, FX-30, FX-29, FX-28
- File based persistence now implemented and default. Dependence on BerekelyDB removed.
- rpmbuild will now create an rpm from fix8.spec
- XML parser accepts embedded spaces between attribute, '=' and attribute value
- Provide mechanism for client or server to set next expected send/receive sequence number
- Client correctly handling sequence_reset from server
- Multiple server sessions write to different logs and persistence files
- Permit applications to by-pass chksum checking
- Chksum logic correctly comparing calculated to passed value
<a id="067"></a>
## [0.6.7] - 2013-01-28
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira issues: FX-17, FX-19, FX-20, FX-21, FX-24, FX-26, FX-27
- Reduction in encode/decode latency by around 29%
<a id="066"></a>
## [0.6.6] - 2012-12-16
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira issues: FX-17, FX-18
<a id="065"></a>
## [0.6.5] - 2012-12-09
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira issues: FX-11, FX-12, FX-13, FX-14, FX-15, FX-16
- Changed to LGPL; added performance script
<a id="064"></a>
## [0.6.4] - 2012-11-22
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira issues: FX-4, FX-10
<a id="063"></a>
## [0.6.3] - 2012-11-18
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed jira issues: FX-1, FX-4, FX-5, FX-6, FX-7, FX-8, FX-9
<a id="062"></a>
## [0.6.2] - 2012-10-23
*Maintainer: David L. Dight <fix@fix8.org>*
- Changed to GPLv2; added shell command pipe variable import to XML; added xi:include to XML;
<a id="061"></a>
## [0.6.1] - 2012-09-07
*Maintainer: David L. Dight <fix@fix8.org>*
- Added configurable max message length(maxmsglen); xml xpath permits absolute xpaths; fixed issue with compiler and msgtype;
<a id="060"></a>
## [0.6.0] - 2012-07-21
*Maintainer: David L. Dight <fix@fix8.org>*
- Added buffered logging; redesigned and improved field and message encoding and decoding performance, field lookups and static jump tables;
- Added simple console based metadata driven test harness; compiler optimisations including pruning of unused fields;
<a id="057"></a>
## [0.5.7] - 2012-05-25
*Maintainer: David L. Dight <fix@fix8.org>*
- Added HF test client demonstrating preload and bulk transmit capabilities; encode/decode performance improvements with cached lookups;
- optimised field generation; fast ascii to int/double;
<a id="055"></a>
## [0.5.5] - 2012-05-12
*Maintainer: David L. Dight <fix@fix8.org>*
- Added support to compiler to generate user session class permitting quicker startup development;
- added presorted_set which significantly reduces Message contruction time; added codec and profile compilation switches;
<a id="051"></a>
## [0.5.1] - 2012-04-29
*Maintainer: David L. Dight <fix@fix8.org>*
- Optimsed compiler, enforce unique fields; updated schemas and documentation.
- Myfix.cpp now uses FIX5.0SP2.
<a id="050"></a>
## [0.5.0] - 2012-04-22
*Maintainer: David L. Dight <fix@fix8.org>*
- Support for FIX5.X and FIXT1.1; Support for nested components and repeating groups; Numerous bug fixes;
- Myfix.cpp now uses FIX5.0SP2.
<a id="0417"></a>
## [0.4.17] - 2012-03-20
*Maintainer: David L. Dight <fix@fix8.org>*
- Postmessage ctor automatically called;
<a id="0416"></a>
## [0.4.16] - 2012-03-08
*Maintainer: David L. Dight <fix@fix8.org>*
- reliable nanosleep;pipelogger,bclogger;gzstream append;bug fixes
<a id="0415"></a>
## [0.4.15] - 2012-03-01
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed static const in class definition storage linker error; fixed superfluous const level return warnings on functions;
- sequence reset flag support; log entries timestamped at creation; threadcode purging; seqnum seeding;
- numerous bugfixes;
<a id="0412"></a>
## [0.4.12] - 2012-02-19
*Maintainer: David L. Dight <fix@fix8.org>*
- Config updated - separate out log files into their own entity; added addiitonal log flags;
- retransmission improved; socket conditioning; sequence number enforcement on login; sequence
- number control at message level; session wrappers; reliable client wrapper; improved multithreading;
- updated myfix to use session wrapper; myfix supports seqnum cmd line args;
<a id="0410"></a>
## [0.4.10] - 2012-02-05
*Maintainer: David L. Dight <fix@fix8.org>*
- Added custom seqnum support; Fixed retransmission behaviour; Added SessionWrapper classes
- to simplify client/server session setup; client connect will retry configurable number of times with interval;
- fixed message field replace, remove and copy_legal; myfix.cpp uses new session wrappers;
- added new config extract methods;
<a id="044"></a>
## [0.4.4] - 2012-01-27
*Maintainer: David L. Dight <fix@fix8.org>*
- Fixed checksum and bodylength calculation bugs on encode and decode;
- Test server permits reconnects and detects logout;
- Connection classes shutdown properly, gracefully;
<a id="042"></a>
## [0.4.2] - 2012-01-23
*Maintainer: David L. Dight <fix@fix8.org>*
- Moved to github; numerous fixes; documentation and wiki;
<a id="03580"></a>
## [0.3.58] - 2012-01-07
*Maintainer: David L. Dight <fix@fix8.org>*
- compiler and basic metadata
<a id="012"></a>
## [0.1.2] - 2010-09-11
*Maintainer: David L. Dight <fix@fix8.org>*
- Initial release on sourceforge
