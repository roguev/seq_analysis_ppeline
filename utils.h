#ifndef __UTILS_H__
#define __UTILS_H__

#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <cstdint>
using namespace std;

namespace utils {

	// generates a regex string for use with the read_extractor module
	string gen_regex_string(
			const string& pat_l,
			const string& pat_r,
			bool mm_l,
			bool mm_r, 
			bool mm_s,
			const vector<uint16_t>& roi_min,
			const vector<uint16_t>& roi_max,
			const vector<string>& spacers);

	// find differences between two strings
	int find_diffs(const string& a, const string& b);

	// splits a string at a specific separator
	vector<string> split_string(const string& str, const string& sep);

	// joins a vector of strings into a single string using a delimiter
	string join_into_string(vector<string>& v, char sep);

	// reverse-complement a sequence
	string seq_revcom(const string& seq);

	// finds a likely match of a string given a library of possible matches
	int find_likely_match(const string& seq, vector<string>& choices, int mm);

	// counts number of bases in a sequence with quality lower than specified quality
	int seq_qual(const string& seq, int q);

	/* attaches a stream to a filename
	 * arguments
	 * 	filename
	 * 	stream reference
	 * 	open_more
	 */
	template<class T>
	void attach_stream(char* fn, T& stream, std::ios_base::openmode mode) {
		if (!fn) {
			report_error(__FILE__,__func__, "Problem with file  (NULL pointer)");
			exit(1);
		}
	
		stream.open(fn, mode);

		if (!stream.good()) {
			string err = "Problem with file " + string(fn);
			report_error(__FILE__, __func__, err);		
			exit(1);
		}
	}

	// general purpose error reporting function
	void report_error(const string& file, const string& func, const string& error_msg);

	// conversta s string to upper case
	void to_upper(string& s);

	// converts a string to lower case
	void to_lower(string& s);

	// replaces a patter in a string with another pattern
	void replace_string_in_place(string& s, const string& search, const string& replace);

} // namespace
#endif // __UTILS_H__
