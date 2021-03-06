
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

#ifndef _yarmi__client__qt_client_base_hpp
#define _yarmi__client__qt_client_base_hpp

#include <yarmi/yarmi_fwd.hpp>

#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <QtNetwork/QTcpSocket>

namespace yarmi {

/***************************************************************************/

struct QtClientBase: private boost::noncopyable {
	QtClientBase(QObject *parent=0);
	virtual ~QtClientBase();

	void connect(const QString &ip, const quint16 port); // may throw if error
	void connect(const QString &ip, const quint16 port, boost::system::error_code &ec);

	void start();
	void disconnect(); // may throw if error
	void disconnect(boost::system::error_code &ec);

	void send(const buffer_pair &buffer);
	virtual void on_received(const buffer_pair &buffer) = 0;

private:
	struct impl;
	impl *pimpl;
};

/***************************************************************************/

} // ns yarmi

#endif // _yarmi__client__qt_client_base_hpp
