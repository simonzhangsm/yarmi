
// Copyright (c) 2013-2016, niXman (i dotty nixman doggy gmail dotty com)
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

#include <yarmi/client/client_base.hpp>
#include <yarmi/serializers/binary_serializer_base.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>

#include <queue>

namespace yarmi {

/***************************************************************************/

struct client_base::impl {
	enum { header_size = binary_serializer_base::header_size() };

	impl(boost::asio::io_service &ios, yarmi::client_base &self)
		:socket(ios)
		,self(self)
		,buffers()
		,write_in_process(false)
	{}

	void disconnect(boost::system::error_code &ec) {
		socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
		if ( ec ) return;
		socket.close(ec);
	}

	void start() {
		read_header();
	}

	void read_header() {
		boost::asio::async_read(
			 socket
			,boost::asio::buffer(header_buffer)
			,std::bind(
				 &client_base::impl::on_header_readed
				,this
				,std::placeholders::_1
				,std::placeholders::_2
			)
		);
	}

	void on_header_readed(const boost::system::error_code &ec, std::size_t rd) {
		if ( ec || rd != header_size )
			YARMI_THROW("ec=%1%, message=%2%", ec.value(), ec.message());

		const std::pair<std::uint32_t, call_id_type> header = binary_serializer_base::unpack_header(header_buffer, header_size);
		if ( header.first == 0 ) {
			self.on_received(header.second, buffer_pair());
			read_header();
		} else {
			::yarmi::buffer_pair buffer = allocate_buffer(header.first);
			boost::asio::async_read(
				 socket
				,boost::asio::buffer(buffer.first.get(), buffer.second)
				,std::bind(
					&client_base::impl::on_body_readed
					,this
					,std::placeholders::_1
					,std::placeholders::_2
					,header.second
					,buffer
				)
			);
		}
	}

	void on_body_readed(
		 const boost::system::error_code &ec
		,const std::size_t rd
		,const yarmi::call_id_type call_id
		,const yarmi::buffer_pair &buffer
	) {
		if ( ec || rd != buffer.second )
			YARMI_THROW("ec=%1%, message=%2%", ec.value(), ec.message());

		self.on_received(call_id, buffer);

		read_header();
	}

	void send(const buffer_pair &buffer) {
		if ( ! write_in_process ) {
			write_in_process = true;

			boost::asio::async_write(
				 socket
				,boost::asio::buffer(buffer.first.get(), buffer.second)
				,std::bind(
					&client_base::impl::sent
					,this
					,std::placeholders::_1
					,std::placeholders::_2
					,buffer
				)
			);
		} else {
			buffers.push(buffer);
		}
	}
	void sent(
		 const boost::system::error_code &ec
		,const std::size_t wr
		,const buffer_pair &buffer
	) {
		write_in_process = false;

		if ( ec || wr != buffer.second )
			YARMI_THROW("ec=%1%, message=%2%", ec.value(), ec.message());

		if ( ! buffers.empty() ) {
			buffer_pair buf = buffers.front();
			buffers.pop();

			send(buf);
		}
	}

	boost::asio::ip::tcp::socket socket;
	yarmi::client_base &self;

	std::queue<buffer_pair> buffers;
	bool write_in_process;

	char header_buffer[header_size];
};

/***************************************************************************/

client_base::client_base(boost::asio::io_service &ios)
	:pimpl(new impl(ios, *this))
{}

client_base::~client_base() {
	boost::system::error_code ec;
	disconnect(ec);

	delete pimpl;
}

/***************************************************************************/

boost::asio::ip::tcp::socket& client_base::get_socket() { return pimpl->socket; }
boost::asio::io_service& client_base::get_io_service() { return pimpl->socket.get_io_service(); }

/***************************************************************************/

void client_base::connect(const std::string &ip, const std::uint16_t port) {
	boost::system::error_code ec;
	connect(ip, port, ec);

	if ( ec )
		YARMI_THROW("ec=%1%, message=%2%", ec.value(), ec.message());
}

void client_base::connect(const std::string &ip, const std::uint16_t port, boost::system::error_code &ec) {
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(ip), port);
	pimpl->socket.connect(ep, ec);
}

/***************************************************************************/

void client_base::start() {
	pimpl->start();
}

/***************************************************************************/

void client_base::disconnect() {
	boost::system::error_code ec;
	pimpl->disconnect(ec);
	if ( ec )
		YARMI_THROW("ec=%1%, message=%2%", ec.value(), ec.message());
}

void client_base::disconnect(boost::system::error_code &ec) {
	pimpl->disconnect(ec);
}

/***************************************************************************/

void client_base::send(const buffer_pair &buffer) {
	pimpl->send(buffer);
}

/***************************************************************************/

} // yarmi
