//-------------------------------------------------------------------------------------------------
#ifndef _IF_FIX_MESSAGE_HPP_
#define _IF_FIX_MESSAGE_HPP_

#include <map>

//-------------------------------------------------------------------------------------------------
namespace FIX {

// all fields present in a message - mandatory and optional
typedef std::map<unsigned short, BaseField *> Fields;

class Message
{
public:
	template<typename InputIterator>
	Message(const InputIterator begin, const InputIterator end) : _fp(begin, end) {}

	Message() {}

	Fields _fields;
	FieldTraits _fp;
};

struct MessageSubElements
{
	static const FieldTrait header_ft[], *header_ft_end, trailer_ft[], *trailer_ft_end;
	Message _header, _trailer;

public:
	MessageSubElements() :
		_header(header_ft, header_ft_end), _trailer(trailer_ft, trailer_ft_end) {}
};

} // FIX

#endif // _IF_FIX_MESSAGE_HPP_
