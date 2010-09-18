// *** f8c generated file: Do Not Edit
//-------------------------------------------------------------------------------------------------
#if 0

Fix8 is released under the New BSD License.

Copyright (c) 2010, David L. Dight <www@orbweb.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of
	 	conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list
	 	of conditions and the following disclaimer in the documentation and/or other
		materials provided with the distribution.
    * Neither the name of the author nor the names of its contributors may be used to
	 	endorse or promote products derived from this software without specific prior
		written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
OR  IMPLIED  WARRANTIES,  INCLUDING,  BUT  NOT  LIMITED  TO ,  THE  IMPLIED  WARRANTIES  OF
MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE ARE  DISCLAIMED. IN  NO EVENT  SHALL
THE  COPYRIGHT  OWNER OR  CONTRIBUTORS BE  LIABLE  FOR  ANY DIRECT,  INDIRECT,  INCIDENTAL,
SPECIAL,  EXEMPLARY, OR CONSEQUENTIAL  DAMAGES (INCLUDING,  BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA,  OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED  AND ON ANY THEORY OF LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR
TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#endif

//-------------------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <iterator>
#include <algorithm>
#include <bitset>
#include <regex.h>
#include <errno.h>
#include <string.h>
// f8 includes
#include <f8exception.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <f8types.hpp>
#include <message.hpp>
#include <myfix_types.hpp>
//-------------------------------------------------------------------------------------------------
namespace FIX8 {
namespace FIX441 {

namespace {

//-------------------------------------------------------------------------------------------------
const std::string MsgType_domain[] = 
   { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "AA", "AB", "AC", "AD", "AE", "AF", "AG", "AH", "AI", "AJ", "AK", "AL", "AM", "AN", "AO", "AP", "AQ", "AR", "AS", "AT", "AU", "AV", "AW", "AX", "AY", "AZ", "B", "BA", "BB", "BC", "BD", "BE", "BF", "BG", "BH", "C", "D", "E", "F", "G", "H", "J", "K", "L", "M", "N", "P", "Q", "R", "S", "T", "U", "U0", "U1", "U10", "U11", "U12", "U13", "U14", "U15", "U16", "U17", "U18", "U19", "U2", "U20", "U3", "U4", "U5", "U6", "U7", "U8", "U9", "V", "W", "X", "Y", "Z", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z" };
const int EncryptMethod_domain[] = 
   { 0, 1, 2, 3, 4, 5, 6 };

//-------------------------------------------------------------------------------------------------
const DomainBase dombases[] =
{
   { reinterpret_cast<const void *>(MsgType_domain), static_cast<DomainBase::DomType>(1), 
      static_cast<FieldTrait::FieldType>(15), 115 },
   { reinterpret_cast<const void *>(EncryptMethod_domain), static_cast<DomainBase::DomType>(1), 
      static_cast<FieldTrait::FieldType>(1), 7 },
};

//-------------------------------------------------------------------------------------------------
BaseField *Create_BeginString(const std::string& from, const BaseEntry *be)
   { return new BeginString(from, be->_dom); }
BaseField *Create_BodyLength(const std::string& from, const BaseEntry *be)
   { return new BodyLength(from, be->_dom); }
BaseField *Create_CheckSum(const std::string& from, const BaseEntry *be)
   { return new CheckSum(from, be->_dom); }
BaseField *Create_MsgSeqNum(const std::string& from, const BaseEntry *be)
   { return new MsgSeqNum(from, be->_dom); }
BaseField *Create_MsgType(const std::string& from, const BaseEntry *be)
   { return new MsgType(from, be->_dom); }
BaseField *Create_SenderCompID(const std::string& from, const BaseEntry *be)
   { return new SenderCompID(from, be->_dom); }
BaseField *Create_SendingTime(const std::string& from, const BaseEntry *be)
   { return new SendingTime(from, be->_dom); }
BaseField *Create_TargetCompID(const std::string& from, const BaseEntry *be)
   { return new TargetCompID(from, be->_dom); }
BaseField *Create_RawDataLength(const std::string& from, const BaseEntry *be)
   { return new RawDataLength(from, be->_dom); }
BaseField *Create_RawData(const std::string& from, const BaseEntry *be)
   { return new RawData(from, be->_dom); }
BaseField *Create_EncryptMethod(const std::string& from, const BaseEntry *be)
   { return new EncryptMethod(from, be->_dom); }
BaseField *Create_HeartBtInt(const std::string& from, const BaseEntry *be)
   { return new HeartBtInt(from, be->_dom); }
BaseField *Create_TestReqID(const std::string& from, const BaseEntry *be)
   { return new TestReqID(from, be->_dom); }
BaseField *Create_ResetSeqNumFlag(const std::string& from, const BaseEntry *be)
   { return new ResetSeqNumFlag(from, be->_dom); }

} // namespace
} // namespace FIX441

//-------------------------------------------------------------------------------------------------
template<>
const FIX441::Myfix::Pair FIX441::Myfix::_pairs[] =
{
   { 8, { &FIX441::Create_BeginString, 0, 0 } },
   { 9, { &FIX441::Create_BodyLength, 0,
      "Length of message excluding header tag" } },
   { 10, { &FIX441::Create_CheckSum, 0, 0 } },
   { 34, { &FIX441::Create_MsgSeqNum, 0, 0 } },
   { 35, { &FIX441::Create_MsgType, &FIX441::dombases[0],
      "FIX message type" } },
   { 49, { &FIX441::Create_SenderCompID, 0, 0 } },
   { 52, { &FIX441::Create_SendingTime, 0, 0 } },
   { 56, { &FIX441::Create_TargetCompID, 0, 0 } },
   { 95, { &FIX441::Create_RawDataLength, 0, 0 } },
   { 96, { &FIX441::Create_RawData, 0, 0 } },
   { 98, { &FIX441::Create_EncryptMethod, &FIX441::dombases[1],
      "Payload encryption method" } },
   { 108, { &FIX441::Create_HeartBtInt, 0,
      "Sent by both sides to ensure connection is alive" } },
   { 112, { &FIX441::Create_TestReqID, 0, 0 } },
   { 141, { &FIX441::Create_ResetSeqNumFlag, 0, 0 } }
};
template<>
const size_t FIX441::Myfix::_pairsz(sizeof(_pairs)/sizeof(FIX441::Myfix));
template<>
const FIX441::Myfix::NoValType FIX441::Myfix::_noval = {0, 0};

} // namespace FIX8
