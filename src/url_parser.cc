/*
 * Copyright (C) 2016 deipi.com LLC and contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "url_parser.h"

#include <cassert>
#include <cstdlib>


std::string urldecode(const char *str, size_t size) {
	char *dStr = new char[size + 1];
	char eStr[] = "00"; /* for a hex code */

	strncpy(dStr, str, size);
	dStr[size] = '\0';

	int i; /* the counter for the string */

	for(i=0;i<strlen(dStr);++i) {

		if(dStr[i] == '%') {
			if(dStr[i+1] == 0)
				return std::string(dStr);

			if(isxdigit(dStr[i+1]) && isxdigit(dStr[i+2])) {

				/* combine the next to numbers into one */
				eStr[0] = dStr[i+1];
				eStr[1] = dStr[i+2];

				/* convert it to decimal */
				long int x = strtol(eStr, NULL, 16);

				/* remove the hex */
				memmove(&dStr[i+1], &dStr[i+3], strlen(&dStr[i+3])+1);

				dStr[i] = x;
			}
		}
		else if(dStr[i] == '+') { dStr[i] = ' '; }
	}

	return std::string(dStr);
}


QueryParser::QueryParser()
	: len(0),
	  off(nullptr) { }


void
QueryParser::clear() noexcept
{
	rewind();
	query.clear();
}


void
QueryParser::rewind() noexcept
{
	len = 0;
	off = nullptr;
}


int
QueryParser::init(const std::string& q)
{
	clear();
	query = q;
	return 0;
}


int
QueryParser::next(const char *name)
{
	const char *ni = query.data();
	const char *nf = ni + query.length();
	const char *n0, *n1 = nullptr;
	const char *v0 = nullptr;

	if (off == nullptr) {
		n0 = n1 = ni;
	} else {
		n0 = n1 = off + len;
	}

	while (true) {
		char cn = *n1;
		if (n1 == nf) {
			cn = '\0';
		}
		switch (cn) {
			case '=' :
				v0 = n1;
			case '\0':
			case '&' :
			case ';' :
				if (strlen(name) == static_cast<size_t>(n1 - n0) && strncmp(n0, name, n1 - n0) == 0) {
					if (v0) {
						const char *v1 = v0 + 1;
						while (1) {
							char cv = *v1;
							if (v1 == nf) {
								cv = '\0';
							}
							switch(cv) {
								case '\0':
								case '&' :
								case ';' :
								off = v0 + 1;
								len = v1 - v0 - 1;
								return 0;
							}
							++v1;
						}
					} else {
						off = n1 + 1;
						len = 0;
						return 0;
					}
				} else if (!cn) {
					return -1;
				} else if (cn != '=') {
					n0 = n1 + 1;
					v0 = nullptr;
				}
		}
		++n1;
	}

	return -1;
}


std::string
QueryParser::get()
{
	if (!off) return std::string();
	return urldecode(off, len);
}


PathParser::PathParser()
	: len_pth(0), off_pth(nullptr),
	  len_hst(0), off_hst(nullptr),
	  len_nsp(0), off_nsp(nullptr),
	  len_pmt(0), off_pmt(nullptr),
	  len_cmd(0), off_cmd(nullptr),
	  len_id(0), off_id(nullptr) { }


void
PathParser::clear() noexcept
{
	rewind();
	len_pmt = 0;
	off_pmt = nullptr;
	len_cmd = 0;
	off_cmd = nullptr;
	len_id = 0;
	off_id = nullptr;
	path.clear();
}


void
PathParser::rewind() noexcept
{
	off = path.data();
	len_pth = 0;
	off_pth = nullptr;
	len_hst = 0;
	off_hst = nullptr;
	len_nsp = 0;
	off_nsp = nullptr;
}


PathParser::State
PathParser::init(const std::string& p)
{
	clear();
	path = p;

	size_t length;
	const char *ni = path.data();
	const char *nf = ni + path.length();
	const char *n0, *n1 = nullptr;
	State state;

	// This first goes backwards, looking for pmt, cmd and id
	// id is filled only if there's no pmt already:
	state = State::START;
	n0 = n1 = nf - 1;
	while (true) {
		char cn = (n1 >= nf || n1 < ni) ? '\0' : *n1;
		switch (cn) {
			case '\0':
			case '/':
				switch (state) {
					case State::START:
						if (!cn) {
							n0 = n1;
							state = State::PMT;
						}
						break;
					case State::PMT:
						assert(n0 >= n1);
						length = n0 - n1;
						if (length && *(n1 + 1) != '_') {
							off_id = n1 + 1;
							len_id = length;
							n0 = n1 - 1;
							state = State::CMD;
							break;
						}
					case State::CMD:
						assert(n0 >= n1);
						length = n0 - n1;
						if (length && *(n1 + 1) == '_') {
							off_pmt = off_id;
							len_pmt = len_id;
							off_id = nullptr;
							len_id = 0;
							off_cmd = n1 + 1;
							len_cmd = length;
							n0 = n1 - 1;
							state = State::ID;
							break;
						}
					case State::ID:
						if (!off_id) {
							assert(n0 >= n1);
							length = n0 - n1;
							if (length) {
								off_id = n1 + 1;
								len_id = length;
							}
						}
						off = ni;
						return state;
					default:
						break;
				}
				break;

			case ',':
			case ':':
			case '@':
				switch (state) {
					case State::START:
						n0 = n1;
						state = State::PMT;
						break;
					case State::ID:
						off = ni;
						return state;
					default:
						break;
				}
				break;

			default:
				switch (state) {
					case State::START:
						n0 = n1;
						state = State::PMT;
						break;
					default:
						break;
				}
				break;
		}
		--n1;
	}

	return state;
}


PathParser::State
PathParser::next()
{
	size_t length;
	const char *ni = path.data();
	const char *nf = ni + path.length();
	const char *n0, *n1 = nullptr;
	State state;

	// Then goes forward, looking for endpoints:
	state = State::NSP;
	off_hst = nullptr;
	n0 = n1 = off;
	if (off_pmt && off_pmt < nf) {
		nf = off_pmt - 1;
	}
	if (off_cmd && off_cmd < nf) {
		nf = off_cmd - 1;
	}
	if (off_id && off_id < nf) {
		nf = off_id - 1;
	}
	if (nf < ni) {
		nf = ni;
	}
	if (n1 > nf) {
		return State::END;
	}
	while (true) {
		char cn = (n1 >= nf || n1 < ni) ? '\0' : *n1;
		switch (cn) {
			case '\0':
			case ',':
				switch (state) {
					case State::NSP:
					case State::PTH:
						assert(n1 >= n0);
						length = n1 - n0;
						off_pth = n0;
						len_pth = length;
						off = ++n1;
						return state;
					case State::HST:
						assert(n1 >= n0);
						length = n1 - n0;
						if (!length) return State::INVALID_HST;
						off_hst = n0;
						len_hst = length;
						off = ++n1;
						return state;
					default:
						break;
				}
				break;

			case ':':
				switch (state) {
					case State::NSP:
						assert(n1 >= n0);
						length = n1 - n0;
						if (!length) return State::INVALID_NSP;
						off_nsp = n0;
						len_nsp = length;
						n0 = n1 + 1;
						state = State::PTH;
						break;
					default:
						break;
				}
				break;

			case '@':
				switch (state) {
					case State::NSP:
					case State::PTH:
						assert(n1 >= n0);
						length = n1 - n0;
						off_pth = n0;
						len_pth = length;
						n0 = n1 + 1;
						state = State::HST;
						break;
					default:
						break;
				}
				break;

			default:
				break;
		}
		++n1;
	}

	return state;
}


std::string
PathParser::get_pth()
{
	if (!off_pth) return std::string();
	return urldecode(off_pth, len_pth);
}


std::string
PathParser::get_hst()
{
	if (!off_hst) return std::string();
	return urldecode(off_hst, len_hst);
}


std::string
PathParser::get_nsp()
{
	if (!off_nsp) return std::string();
	return urldecode(off_nsp, len_nsp);
}


std::string
PathParser::get_pmt()
{
	if (!off_pmt) return std::string();
	return urldecode(off_pmt, len_pmt);
}


std::string
PathParser::get_cmd()
{
	if (!off_cmd) return std::string();
	return urldecode(off_cmd, len_cmd);
}


std::string
PathParser::get_id()
{
	if (!off_id) return std::string();
	return urldecode(off_id, len_id);
}