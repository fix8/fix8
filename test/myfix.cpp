//-----------------------------------------------------------------------------------------
#include <iostream>
#include <memory>
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

// f8 headers
#include <f8exception.hpp>
#include <f8utils.hpp>
#include <traits.hpp>
#include <field.hpp>
#include <f8types.hpp>
#include <message.hpp>
#include "Myfix_types.hpp"

//-----------------------------------------------------------------------------------------
using namespace std;
using namespace FIX8;

//-----------------------------------------------------------------------------------------
static const std::string rcsid("$Id: f8c.cpp 515 2010-09-16 01:13:48Z davidd $");

//-----------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	cout << "Version: " << SFE::Myfix::get_version() << endl;

	try
	{
		const BaseEntry& be(SFE::Myfix::find_ref(35));
		//const BaseEntry& be(SFE::Myfix::find_ref(101));
		string logon("A");
		auto_ptr<BaseField> fld(be._create(logon, &be));
		if (be._comment)
			cout << be._comment << ": ";
		cout << *fld << endl;
		cout << "is valid: " << boolalpha << fld->isValid() << endl << endl;
	}
	catch (InvalidMetadata& ex)
	{
		cerr << ex.what() << endl;
	}

	FieldTrait::TraitBase a = { 1, FieldTrait::ft_int, 0, true, false, false };

	const BaseEntry& be1(SFE::Myfix::find_ref(98));
	string encryptmethod("101");
	auto_ptr<BaseField> fld1(be1._create(encryptmethod, &be1));
	if (be1._comment)
		cout << be1._comment << ": ";
	cout << *fld1 << endl;
	cout << "is valid: " << boolalpha << fld1->isValid() << endl;
	//SFE::EncryptMethod& em(dynamic_cast<SFE::EncryptMethod&>(*fld1));
	//cout << em.get() << endl;
	return 0;
}

