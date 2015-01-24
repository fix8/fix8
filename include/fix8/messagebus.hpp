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
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_MESSAGEBUS_HPP_
#define FIX8_MESSAGEBUS_HPP_

//-------------------------------------------------------------------------------------------------
namespace FIX8
{

//-------------------------------------------------------------------------------------------------
namespace MBUS
{

//-------------------------------------------------------------------------------------------------
/// Base class for our message
class GenericMessage
{
public:
	/*! Ctor. */
	GenericMessage() {}

	/// Dtor.
	virtual ~GenericMessage() {}
};

//-------------------------------------------------------------------------------------------------
/// Base class for our subscriber
class MessageBusSubscriber
{
public:
	/*! Ctor. */
	MessageBusSubscriber() {}

	/// Dtor.
	virtual ~MessageBusSubscriber() {}

	/*! Callback method on receipt of message */
	virtual bool receive_message(const GenericMessage *msg) { return false; }
};

//-------------------------------------------------------------------------------------------------
/// Encapsulates a message bus context
class MessageBus
{
public:
	/*! Ctor. */
	MessageBus() {}

	/// Dtor.
	virtual ~MessageBus() {}

	/*! Starts the messaging subsystem */
	virtual bool start() { return true; }

	/*! Stops the messaging subsystem */
	virtual bool stop() { return true; }

	/*! Publishes a message to all aubscribers for given topic*/
	virtual bool publish(const f8String& topic, const GenericMessage *msg) { return false; }

	/*! Creates a subscription for the given topic */
	virtual bool subscribe(MessageBusSubscriber *subscriber, const f8String& topic) { return false; }

	/*! Creates a subscription for the given topic, returns new subscriber */
	virtual MessageBusSubscriber *subscribe(const f8String& topic) { return nullptr; }

	/*! Removes a subscription for a given subscriber */
	virtual bool unsubscribe(MessageBusSubscriber *subscriber, const f8String& topic) { return false; }
};

//-------------------------------------------------------------------------------------------------

} // MBUS
} // FIX8

#endif // FIX8_MESSAGEBUS_HPP_
