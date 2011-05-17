//
// client_msg_parser.hpp
// ~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Victor C. Su
//

#ifndef SLIM_CLIENT_MSG_PARSER_HPP
#define SLIM_CLIENT_MSG_PARSER_HPP

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>

namespace slim {
namespace server {

struct client_msg;

/// Parser for incoming client messages.
class client_msg_parser
{
public:
	/// Construct ready to parse the client message.
	client_msg_parser();

	/// Reset to initial parser state.
	void reset();

	/// Parse some data. The tribool return value is true when a complete request
	/// has been parsed, false if the data is invalid, indeterminate when more
	/// data is required. The InputIterator return value indicates how much of the
	/// input has been consumed.
	template <typename InputIterator>
	boost::tuple<boost::tribool, InputIterator> parse(client_msg& c_msg,
		InputIterator begin, InputIterator end)
	{
		while (begin != end)
		{
			boost::tribool result = consume(c_msg, *begin++);
			if (result || !result)
				return boost::make_tuple(result, begin);
		}

		boost::tribool result = boost::indeterminate;
		return boost::make_tuple(result, begin);
	}	

private:
	/// Handle the next character of input.
	boost::tribool consume(client_msg& c_msg, char input);

	/// The message byte index.
	int index_;
};

} // namespace slim
} // namespace http

#endif // SLIM_CLIENT_MSG_PARSER_HPP