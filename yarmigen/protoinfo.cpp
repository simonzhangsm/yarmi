
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

#include "protoinfo.hpp"

#include <ostream>
#include <iomanip>

namespace yarmigen {

void proc_info::dump(std::ostream &os) const {
	enum { max_len = 32 };

	auto fill_by_spaces = [](std::ostream &os, std::size_t num) {
		for ( ; num; --num )
			os << ' ';
	};

	os << '\'' << request << '\'';
	fill_by_spaces(os, max_len - request.length());

	os << " -> '" << handler << '\'';
	fill_by_spaces(os, max_len - handler.length());

	os << ": (";
	for ( auto it = args.begin(); it != args.end(); ++it ) {
		os << *it << (it+1 != args.end() ? ", " : "");
	}
	os << ')';
}

void proto_info::dump(std::ostream &os) const {
	os
	<< "type             : " << (type_ == info_type::api ? "api" : "service") << std::endl;
	if ( type_ == info_type::api ) {
		os
		<< "client namespace : " << req_namespace_ << std::endl
		<< "client class name: " << req_class_ << std::endl;
		std::size_t idx = 0;
		for ( const auto &it: req_procs_ ) {
			os << "  " << std::setw(2) << std::setfill('0') << idx++ << ": ";
			it.dump(os);
			os << std::endl;
		}

		os
		<< "server namespace : " << rep_namespace_ << std::endl
		<< "server class name: " << rep_class_ << std::endl;
		idx = 0;
		for ( const auto &it: rep_procs_ ) {
			os << "  " << std::setw(2) << std::setfill('0') << idx++ << ": ";
			it.dump(os);
			os << std::endl;
		}
	} else {

	}
}

} // ns yarmigen