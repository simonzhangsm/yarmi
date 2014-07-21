
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

#include <yarmi/serialization.hpp>
#include <yarmi/session_base.hpp>
#include <yarmi/server_base.hpp>
#include <yarmi/global_context_base.hpp>
#include <yarmi/handler_allocator.hpp>
#include <yarmi/make_preallocated_handler.hpp>
#include <yarmi/throw.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <queue>
#include <functional>

namespace yarmi {

/***************************************************************************/

struct session_base::impl {
	enum { header_size = sizeof(std::uint32_t)+::yarmi::iarchive_type::header_size() };

	impl(server_base &sb, global_context_base &gcb)
		:gcb(gcb)
		,stat(sb.get_server_statistic())
		,socket(sb.get_io_service())
		,eh(sb.get_error_handler())
		,buffers()
		,in_process(false)
	{}

	void read_header(session_ptr self) {
		boost::asio::async_read(
			 socket
			,boost::asio::buffer(header_buffer)
			,yarmi::make_preallocated_handler(
				read_allocator
				,std::bind(
					&impl::header_readed
					,this
					,std::placeholders::_1
					,std::placeholders::_2
					,self
				)
			)
		);
	}

	void header_readed(
		 const boost::system::error_code &ec
		,const std::size_t rd
		,session_base::session_ptr self
	) {
		if ( ec || rd != header_size ) {
			eh(YARMI_FORMAT_MESSAGE_AS_STRING("header read error: \"%1%\"", ec.message()));
			return;
		}

		stat.readed += rd;
		stat.read_rate += rd;

		::yarmi::istream_type is(header_buffer, header_size);
		::yarmi::iarchive_type ia(is);
		std::uint32_t body_length = 0;
		ia & body_length;

		std::shared_ptr<char> body_buffer(new char[body_length], [](char *ptr){delete []ptr;});
		boost::asio::async_read(
			 socket
			,boost::asio::buffer(body_buffer.get(), body_length)
			,yarmi::make_preallocated_handler(
				read_allocator
				,std::bind(
					&impl::body_readed
					,this
					,std::placeholders::_1
					,std::placeholders::_2
					,self
					,body_buffer
					,body_length
				)
			)
		);
	}
	void body_readed(
		 const boost::system::error_code &ec
		,const std::size_t rd
		,session_base::session_ptr self
		,std::shared_ptr<char> buffer
		,std::size_t buffer_size
	) {
		if ( ec || rd != buffer_size ) {
			eh(YARMI_FORMAT_MESSAGE_AS_STRING("body read error: \"%1%\"", ec.message()));
			return;
		}

		stat.readed += rd;
		stat.read_rate += rd;
		++stat.read_ops;

		try {
			self->on_received(buffer.get(), rd);
		} catch (const std::exception &ex) {
			eh(YARMI_FORMAT_MESSAGE_AS_STRING("exception is thrown when invoking: \"%1%\"", ex.what()));
		}

		read_header(self);
	}

	void send(session_base::session_ptr self, const yas::shared_buffer &buffer) {
		if ( !in_process ) {
			in_process = true;

			stat.write_queue_size = buffers.size()+1;

			boost::asio::async_write(
				 socket
				,boost::asio::buffer(buffer.data.get(), buffer.size)
				,yarmi::make_preallocated_handler(
					write_allocator
					,std::bind(
						&impl::sent
						,this
						,std::placeholders::_1
						,std::placeholders::_2
						,self
						,buffer
					)
				)
			);
		} else {
			buffers.push(buffer);
			stat.write_queue_size = buffers.size();
		}
	}

	void sent(
		 const boost::system::error_code &ec
		,const std::size_t wr
		,session_base::session_ptr self
		,yas::shared_buffer buffer
	) {
		in_process = false;

		if ( ec || wr != buffer.size ) {
			eh(YARMI_FORMAT_MESSAGE_AS_STRING("write error: \"%1%\"", ec.message()));
		}

		stat.writen += wr;
		stat.write_rate += wr;
		++stat.write_ops;
		--stat.write_queue_size;

		if ( ! buffers.empty() ) {
			yas::shared_buffer buffer = buffers.front();
			buffers.pop();

			send(self, buffer);
		}
	}

	yarmi::handler_allocator<512> read_allocator;
	yarmi::handler_allocator<512> write_allocator;

	global_context_base &gcb;
	server_statistic &stat;
	boost::asio::ip::tcp::socket socket;

	server_base::error_handler_type eh;

	/** buffers list */
	std::queue<yas::shared_buffer> buffers;
	bool in_process;

	char header_buffer[header_size];
}; // struct session_base::impl

/***************************************************************************/

session_base::session_base(server_base &sb, global_context_base &gcb)
	:pimpl(new impl(sb, gcb))
{}

session_base::~session_base()
{ delete pimpl; }

/***************************************************************************/

boost::asio::ip::tcp::socket &session_base::get_socket()
{ return pimpl->socket; }

/***************************************************************************/

void session_base::start() {
	pimpl->read_header(shared_from_this());
}

/***************************************************************************/

void session_base::stop() { pimpl->socket.cancel(); }
void session_base::close() { pimpl->socket.close(); }

/***************************************************************************/

void session_base::send(const yas::shared_buffer &buffer) {
	pimpl->send(shared_from_this(), buffer);
}

/***************************************************************************/

} // ns yarmi
