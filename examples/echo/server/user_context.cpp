
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

#include "user_context.hpp"
#include "global_context.hpp"

/***************************************************************************/

user_context::user_context(boost::asio::io_service &ios, global_context<user_context> &gc)
	:yarmi::session_base(ios)
	,yarmi::server_invoker<user_context>(*this, *this)
	,gc(gc)
{}

/***************************************************************************/

void user_context::on_connected() {
	std::cout << "YARMI: on_connected(" << get_socket().remote_endpoint().address().to_string() << ") called" << std::endl;
}

void user_context::on_disconnected() {
	std::cout << "YARMI: on_disconnected() called" << std::endl;
}

void user_context::on_received(const char *ptr, std::size_t size) {
	try {
		yarmi::id_type call_id = 0;
		const bool ok = invoke(ptr, size, &call_id);
		if ( ! ok ) {
			std::cerr << "user_context::on_received(): no handler for call_id=" << call_id << std::endl;
		}
	} catch (const std::exception &ex) {
		std::cerr << "[exception]: " << __PRETTY_FUNCTION__ << ": " << ex.what() << std::endl;
	}
}

/***************************************************************************/

void user_context::on_ping(const std::string &msg) {
	//std::cout << "received: \"" << msg << "\"" << std::endl;
	pong(msg);
	//throw std::runtime_error("remote exception");
}

/***************************************************************************/
