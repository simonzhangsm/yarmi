
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

#include "server_first_invoker.hpp"
#include "global_context.hpp"
#include "user_context.hpp"

#include <yarmi/session_base.hpp>

//#include <iostream>

namespace two_invokers {

/***************************************************************************/

struct server_first_invoker_impl::impl {
	impl(user_context &uc, global_context<user_context> &gc)
		:uc(uc)
		,gc(gc)
	{}

	user_context &uc;
	global_context<user_context> &gc;
}; // struct server_first_invoker_impl::impl

/***************************************************************************/

server_first_invoker_impl::server_first_invoker_impl(user_context &uc, global_context<user_context> &gc)
	:server_first_invoker<server_first_invoker_impl, yarmi::session_base>(*this, uc)
	,pimpl(new impl(uc, gc))
{}

server_first_invoker_impl::~server_first_invoker_impl()
{ delete pimpl; }

/***************************************************************************/

void server_first_invoker_impl::on_ping(const std::string &str) {
	//std::cout << "server_first_invoker_impl::on_ping(" << str << ")" << std::endl;
	pimpl->uc.second.pong(str);
}

/***************************************************************************/

} // ns two_invokers