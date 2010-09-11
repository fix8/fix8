#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <algorithm>
#include <bitset>

#include <field.hpp>
#include <traits.hpp>
#include <message.hpp>

//-------------------------------------------------------------------------------------------------
namespace FIX {

typedef Field<int, 34> MsgSeqNum;
typedef Field<std::string, 55> Symbol;
typedef Field<double, 44> Price;

class NewOrderSingle : public Message
{
   static const FieldTrait NewOrderSingle_ft[], *NewOrderSingle_ft_end;
   MessageSubElements _subels;

public:
   NewOrderSingle() : Message(NewOrderSingle_ft, NewOrderSingle_ft_end) {}
};

const FieldTrait NewOrderSingle::NewOrderSingle_ft[] =
{
   FieldTrait(11,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(1,     FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(100,   FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(38,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(40,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(44,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(18,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(81,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(58,    FieldTrait::fst_Length, 0, true, false, false),
   FieldTrait(126,   FieldTrait::fst_Length, 0, true, false, false),
}, *NewOrderSingle::NewOrderSingle_ft_end(NewOrderSingle::NewOrderSingle_ft + sizeof(NewOrderSingle::NewOrderSingle_ft)/sizeof(FieldTrait));

}; // FIX

//-------------------------------------------------------------------------------------------------
using namespace std;

int main()
{
   vector<FIX::BaseField *> fields;
   fields.push_back(new FIX::MsgSeqNum(100));
   fields.push_back(new FIX::Symbol("BHP"));
   fields.push_back(new FIX::MsgSeqNum(100));
   fields.push_back(new FIX::Symbol("BHP"));
   fields.push_back(new FIX::Price(44.37));

   for(vector<FIX::BaseField *>::const_iterator itr(fields.begin()); itr != fields.end(); ++itr)
      cout << **itr << endl;

   return 0;
}

