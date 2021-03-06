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

#pragma once

#include "../src/xapiand.h"

#include <fstream>
#include <sstream>
#include <vector>


#ifndef TESTING_LOGS
#  define TESTING_LOGS 1
#endif
#ifndef TESTING_ENDPOINTS
#  define TESTING_ENDPOINTS 1
#endif
#ifndef TESTING_DATABASE
#  define TESTING_DATABASE 1
#endif


#define TEST_VERBOSITY 3
#define TEST_DETACH false
#define TEST_CHERT false
#define TEST_SOLO true
#define TEST_REQUIRED_TYPE false
#define TEST_OPTIMAL false
#define TEST_DATABASE ""
#define TEST_CLUSTER_NAME "cluster_test"
#define TEST_NODE_NAME "node_test"
#define TEST_PIDFILE ""
#define TEST_LOGFILE ""
#define TEST_UID ""
#define TEST_GID ""
#define TEST_DISCOVERY_GROUP ""
#define TEST_RAFT_GROUP ""
#define TEST_NUM_SERVERS 1
#define TEST_DBPOOL_SIZE 1
#define TEST_NUM_REPLICATORS 1
#define TEST_THREADPOOL_SIZE 1
#define TEST_ENDPOINT_LIST_SIZE 1
#define TEST_NUM_COMMITERS 1
#define TEST_EV_FLAG 0
#define TEST_LOCAL_HOST "127.0.0.1"


#include "../src/utils.h"


#if (TESTING_LOGS == 1)
#  include "../src/log.h"
#  define RETURN(x) { Log::finish(); return x; }
#  define INIT_LOG Log::handlers.push_back(std::make_unique<StderrLogger>());
#else
template <typename... Args>
inline void log(std::string fmt, Args&&... args) {
	fmt.push_back('\n');
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
	fprintf(stderr, fmt.c_str(), std::forward<Args>(args)...);
#pragma GCC diagnostic pop
}
#  define L_TEST(obj, args...) log(args)
#  define L_DEBUG(obj, args...) log(args)
#  define L_INFO(obj, args...) log(args)
#  define L_NOTICE(obj, args...) log(args)
#  define L_WARNING(obj, args...) log(args)
#  define L_ERR(obj, args...) log(args)
#  define L_CRIT(obj, args...) log(args)
#  define L_ALERT(obj, args...) log(args)
#  define L_EMERG(obj, args...) log(args)
#  define L_EXC(obj, args...) log(args)
#  define RETURN(x) { return x; }
#  define INIT_LOG
#endif


#if (TESTING_ENDPOINTS == 1)
#include "../src/endpoint.h"

inline Endpoint create_endpoint(const std::string& database) {
	Endpoint e(database, nullptr, -1, TEST_NODE_NAME);
	e.port = XAPIAND_BINARY_SERVERPORT;
	e.host.assign(TEST_LOCAL_HOST);
	return e;
}
#endif


bool write_file_contents(const std::string& filename, const std::string& contents);
bool read_file_contents(const std::string& filename, std::string* contents);


#if (TESTING_DATABASE == 1) || (TESTING_ENDPOINTS == 1)
#include "../src/manager.h"
#include "../src/database_handler.h"


/*
 *	The database used in the test is local
 *	so the Endpoints and local_node are manipulated.
 */

struct DB_Test {
	DatabaseHandler db_handler;
	std::string name_database;
	Endpoints endpoints;

	DB_Test(const std::string& db_name, const std::vector<std::string>& docs, int flags, const std::string& ct_type=JSON_CONTENT_TYPE);
	~DB_Test();

	void create_manager();
	std::pair<std::string, MsgPack> get_body(const std::string& body, const std::string& ct_type);
};
#endif
