// Copyright (c) 2013,2014, niXman (i dotty nixman doggy gmail dotty com)
// All rights reserved.
//
// This file is part of YARMI(https://github.com/niXman/yarmi) project.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
//   Redistributions in binary form must reproduce the above copyright notice, this
//   list of conditions and the following disclaimer in the documentation and/or
//   other materials provided with the distribution.
//
//   Neither the name of the {organization} nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
// ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _yarmi__server_hpp
#define _yarmi__server_hpp

#include <yarmi/session_base.hpp>
#include <yarmi/handler_allocator.hpp>
#include <yarmi/make_preallocated_handler.hpp>
#include <yarmi/throw.hpp>

#include <boost/noncopyable.hpp>

#include <cstdint>

#include <string>
#include <sstream>

/***************************************************************************/

namespace yarmi {
namespace detail {

static bool default_on_connected_predicate(const boost::asio::ip::tcp::endpoint &) {
	return true;
}
static void default_error_handler(const std::string &msg) {
	std::cerr << msg << std::endl << std::flush;
}

} // ns detail

/***************************************************************************/

template<
	 typename UC
	,template<typename UC> class GC
	,typename CP = bool(*)(const boost::asio::ip::tcp::endpoint &)
	,typename EH = void(*)(const std::string &)
>
struct server: private boost::noncopyable {
	server(
		 const std::string &ip
		,std::uint16_t port
		,boost::asio::io_service &ios
		,GC<UC> &gc
		,CP cp = &detail::default_on_connected_predicate
		,EH eh = &detail::default_error_handler
	)
		:acceptor(ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(ip), port))
		,allocator()
		,gc(gc)
		,cp(cp)
		,eh(eh)
	{}

	void start() {
		auto session = session_base::create<UC>(
			acceptor.get_io_service()
			,gc
			,std::bind(&server<UC, GC>::session_deleter, this, std::placeholders::_1)
		);

		acceptor.async_accept(
			 session->get_socket()
			,yarmi::make_preallocated_handler(
				 allocator
				,std::bind(&server<UC, GC>::on_accepted, this, std::placeholders::_1, session)
			)
		);
	}

private:
	void session_deleter(session_base *session) {
		session->set_on_destruction(true);
		std::ostringstream os;

		YARMI_TRY(on_disconnected_flag)
			session->on_disconnected();
		YARMI_CATCH_LOG(on_disconnected_flag, os, eh(os.str());)

		if ( ! gc.has_session(session) ) {
			eh(YARMI_FORMAT_MESSAGE_AS_STRING("session \"%1%\" not in connected sessions list", session));
		} else {
			gc.del_session(session);
		}

		YARMI_TRY(delete_session_flag)
			delete session;
		YARMI_CATCH_LOG(delete_session_flag, os, eh(os.str());)
	}

	void on_accepted(const boost::system::error_code &ec, session_base::session_ptr session) {
		if ( ! ec ) {
			boost::system::error_code ec2;
			const boost::asio::ip::tcp::endpoint &ep = session->get_socket().remote_endpoint(ec2);
			if ( ec2 ) {
				eh(YARMI_FORMAT_MESSAGE_AS_STRING("cannot get remote endpoint: \"%1%\"", ec2.message()));
			}

			if ( ! cp(ep) ) {
				eh(YARMI_FORMAT_MESSAGE_AS_STRING("IP \"%1%\" is in backlist", ep.address().to_string()));
			} else {
				std::ostringstream os;
				gc.add_session(session.get());

				YARMI_TRY(on_connected_flag)
					session->on_connected();
				YARMI_CATCH_LOG(on_connected_flag, os, eh(os.str());)

				/** start session */
				session->start();
			}

			/** start accepting next connection */
			start();
		}
	}

private:
	boost::asio::ip::tcp::acceptor acceptor;
	yarmi::handler_allocator<512> allocator;
	GC<UC> &gc;

	CP cp;
	EH eh;
};

/***************************************************************************/

} // ns yarmi

#endif // _yarmi__server_hpp
