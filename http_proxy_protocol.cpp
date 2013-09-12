//
//  http_proxy_protocol.cpp
//  coroserver
//
//  Created by Chen Xu on 13-9-11.
//  Copyright (c) 2013 0d0a.com. All rights reserved.
//

#include <iostream>
#include <vector>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include "http_proxy_protocol.h"

bool http_proxy_protocol(std::iostream &s, boost::asio::yield_context yield) {
    using namespace std;
    using namespace boost;
    string line;
    // Read request line
    getline(s, line);
    // TODO: Parse request line
    //typedef vector< iterator_range<string::iterator> > find_vector_type;
    //find_vector_type FindVec;
    //find_all(FindVec, line, " ");
    // Read headers
    do {
        getline(s, line);
        trim(line);
        // TODO: Parse headers
    } while (!line.empty());
    // TODO: 
    s << "HTTP/1.1 200 OK\n";
    s << "Content-Type: text/html\n";
    s << "Content-Length: 65\n";
    s << "\n";
    s << "<HTML>\n<TITLE>Test</TITLE><BODY><H1>It works!</H1></BODY></HTML>\n";
    s.flush();
    return false;
}
