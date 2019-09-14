#ifndef __GZBOOST_H__
#define __GZBOOST_H__

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <string>
using namespace std;

string compress_string(const string& data);
string decompress_string(const string& data);

#endif // __GZBOOST_H__
