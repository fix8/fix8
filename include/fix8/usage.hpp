//-----------------------------------------------------------------------------------------
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

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
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
//-----------------------------------------------------------------------------------------
#ifndef _USAGE_HPP_
# define _USAGE_HPP_

//-----------------------------------------------------------------------------------------
/// Convenient program help/usage wrapper. Generates a standardised usage message.
class UsageMan
{
	const std::string prognm_, params_;
	std::string argstr_, description_;
	using OPTEL = std::map<const char, const std::pair<const std::string, const std::string>>;
	OPTEL optels_;
	std::list<std::string> xtrlines_;
	const int splen_, argoptlen_;

public:
	/*! Ctor.
	  \param prognm program name
	  \param argstr program arguments string
	  \param params program non-switched arguments string
	  \param splen tab width
	  \param argoptlen option argument width */
	UsageMan(const std::string& prognm, const std::string& argstr, const std::string& params,
		int splen=3, int argoptlen=23)
		: prognm_(prognm), params_(params), argstr_(argstr), splen_(splen), argoptlen_(argoptlen)
	{
		std::sort(argstr_.begin(), argstr_.end());
	}

	/// Dtor.
	virtual ~UsageMan() {}

	/*! Add a command line option.
	  \param sw the single character switch
	  \param lsw the string switch (long version)
	  \param help the associated help string
	  \return true on success */
	bool add(const char sw, const std::string& lsw, const std::string& help)
	{
		return optels_.insert({sw, std::pair<const std::string, const std::string>(lsw, help)}).second;
	}

	/*! Add an extra help line. Lines prefixed with '@' are indented one tabstop.
	  \param xtr the extra help line */
	void add(const std::string& xtr)
	{
		std::string topush(xtr);
		std::string::size_type spos;
		while ((spos = topush.find_first_of('@')) != std::string::npos)
			topush.replace(spos, 1, std::string(splen_, ' '));
		xtrlines_.push_back(topush);
	}

	/*! Set the usage description string.
	  \param desc the description string */
	void setdesc(const std::string& desc) { description_ = desc; }

	/*! Insert the formatted usage into the given output stream.
	  \param os the stream to insert to */
	void print(std::ostream& os) const
	{
		if (!description_.empty())
			os << description_ << std::endl << std::endl;
		os << "Usage: " << prognm_ << " [-";
		for (std::string::const_iterator itr(argstr_.begin()); itr != argstr_.end(); ++itr)
			if (*itr != ':')
				os << *itr;
		os << "] " << params_ << std::endl;
		const std::string spacer(splen_, ' ');
		for (OPTEL::const_iterator itr(optels_.begin()); itr != optels_.end(); ++itr)
		{
			std::ostringstream ostr;
			ostr << '-' << itr->first << ",--" << itr->second.first;
			os << spacer << std::left << std::setw(argoptlen_) << ostr.str()
				<< ' ' << itr->second.second << std::endl;
		}
		for (std::list<std::string>::const_iterator itr(xtrlines_.begin()); itr != xtrlines_.end(); ++itr)
			os << *itr << std::endl;
	}
};

#endif // _USAGE_HPP_

