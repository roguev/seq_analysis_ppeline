#include <string>
#include <boost/regex.hpp>
#include <pcrecpp.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include "gzstream.h"
#include "utils.h"
using namespace std;

/* converts a string to upper case
 * arguments
 * 	string
 * 	*/
void utils::to_upper(string& s) {
	for (size_t i = 0; i < s.length(); ++i) {
		s[i] &= ~0x20;
	}
}

/* converts a string to lower case
 * arguments
 * 	string
 * 	*/
void utils::to_lower(string& s) {
	for (size_t i = 0; i < s.length(); ++i) {
		s[i] |= 0x20;
	}
}

/* replaces a pattern in a string with another pattern in place
 * arguments
 * 	string
 * 	string
 * 	string
 * 	*/
void utils::replace_string_in_place(string& s, const string& search, const string& replace) {
	size_t pos = 0;
	while ((pos = s.find(search, pos)) != std::string::npos) {
		s.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

/* generates a regular extression string 
 * arguments:
 * 	string left anchor sequence
 * 	string right anchor sequence
 * 	bool allow mismatch left anchor
 * 	bool allow mismatch right anchor
 * 	bool allow mismatch in spacer
 * 	vector<uint16_t> min lenght of ROI
 * 	vector<uint16_t> max length of ROI
 *	vector<string> spacer sequences
 * 	*/
string utils::gen_regex_string(
		const string& pat_l,
		const string& pat_r,
		bool mm_l,
		bool mm_r,
		bool mm_s,
		const vector<uint16_t>& roi_min,
		const vector<uint16_t>& roi_max,
		const vector<string>& spacers) {

	stringstream ss;
	string temp, temp1 , roi;
	string re;
	vector<string> rois;
	vector<string> sps;

	// mismatch pattern
	string mm = "[ATGCN]";
	string x = "x";

	// populate the rois vector
	// capturing group
	// ([ATGCN]{m,n})
	for (size_t i = 0; i < roi_min.size(); ++i) {
		// reset the stream
		ss.str("");
		ss.clear();
		ss << "(" + mm + "{";
		ss << roi_min.at(i);
		ss << ",";
		ss << roi_max.at(i);
		ss << "})";
		ss >> roi;
		rois.push_back(roi);
	}

	// build spacer part with or w/o mismatches
	// non-capturing group
	// (?:spacer|...|...)
	// <?:spacer>
	for (size_t i = 0; i < spacers.size(); ++i) {
		temp = spacers.at(i);
		if (mm_s) {
			for (size_t j = 0; j < spacers.at(i).length(); ++j) {
				temp1 = spacers.at(i);
				temp1.replace(j,1,mm);
				temp += "|" + temp1;
			}
		}

		temp = "(?:" + temp + ")";
//		cout << temp << endl;
		sps.push_back(temp);
	}

	// build left anchor with or w/o mismatches and add to pattern
	// non-capturing group
	// (?:pat_l|...|....)
	// <?:pat_l>
	re += "(?:" + pat_l;
	if (mm_l) {
		for (size_t i = 0; i < pat_l.length(); ++i) {
			temp = pat_l;
			temp.replace(i,1,mm);
			re += "|" + temp;
//			cout << re << endl;
		}
	}

	re += ")";

//	cout << re << endl;

	// build the roi + spacer part and add to pattern
	// (?:pat_l|...|...)([ATGCN]{m,n})(?:spacer|....([ATGCN]{m,n})(?:spacer|...)([ATGCN]{m,n})
	// <?:pat_l> <?:spacer1> <roi1> <?:spacer2> <roi2> <?:spacerN> <roiN>
//	cout << re << endl;
	if (sps.size() > 0) {
		re += rois.front();
		for (size_t i = 0; i < rois.size() - 1 ; ++i) {
			re += sps.at(i) + rois.at(i+1);
			}
//		cout << re << endl;
	} else {
		for (size_t i = 0; i < rois.size(); ++i) {
			re += rois.at(i);
		}
	}

	// build right anchor with or w/o mismatches and add to pattern
	// (pat_l|...|...) ([ATGCN]{m,n})(spacer|....([ATGCN]{m,n})(spacer|...)([ATGCN]{m,n}) (pat_r|...|...)
	// <?:pat_l> <?:spacer1> <roi1> <?:spacer2> <roi2> <?:spacerN> <roiN> <?:pat_r>
	re += "(?:" + pat_r;
//	cout << re << endl;
	if (mm_r) {
		for (size_t i = 0; i < pat_r.length(); ++i) {
			temp = pat_r;
			temp.replace(i,1,mm);
			re += "|" + temp;
//			cout << re << endl;
		}
	}
	re += ")";

	replace_string_in_place(re, x, mm);

//	cout << re << endl;

	return re;
}

/* finds number of differences between 2 strings
 * arguments:
 * 	2 strings to be compared
 * 	*/
int utils::find_diffs(const string& a, const string& b) {
	int res = 0;
	for (size_t i = 0; i < a.length(); ++i) {
		if (a.at(i) != b.at(i)) {res++; }
	}
	return res;
}

/* splits a string into a vector using a delimiter
 * arguments:
 * 	string to be split
 * 	char delimiter
 * 	*/
vector<string> utils::split_string(const string& str, const string& sep) {
	vector<string> res;
	for (size_t p = 0, q = 0; p!= str.npos; p = q) {
		res.push_back(str.substr(p+sep.size()*(p!=0), (q=str.find(sep, p+1)) - p - sep.size()*(p!=0)));
	}
	return res;
}

/* joins a string vector into a string using a delimiter
 * arguments:
 * 	vector of strings to be stitched together
 * 	char delimiter
 * 	*/
string utils::join_into_string(vector<string>& v, char sep) {
	string res;
	for (size_t i = 0; i < v.size(); ++i) {
		res += v.at(i);
		res.push_back(sep);
	}
	res.erase(res.length() - 1);
	return res;
}

/* get reverse complement of a sequence
 * arguments:
 * 	string sequence
 * 	*/
string utils::seq_revcom(const string& seq) {
	string res = seq;
	// convert to upper case
	to_upper(res);

	// reverse sequence
	reverse(res.begin(), res.end());
	
	//iterate over bases and complement
	for (string::iterator it = res.begin(); it != res.end(); ++it) {
		switch(*it) {
			case('A'): case('a'): *it = 'T'; break;
			case('T'): case('t'): *it = 'A'; break;
			case('G'): case('g'): *it = 'C'; break;
			case('C'): case('c'): *it = 'G'; break;
		}
	}
	return res;
}

/* counts number of bases with quality lower than a specified quality
 * arguments:
 * 	string sequence
 * 	int minimum quality
 * 	*/
int utils::seq_qual(const string& seq, int q) {
	int res = 0;
	for (string::const_iterator it = seq.begin(); it != seq.end(); ++it) {
		// convert ASCII representation of quality into a number
		if (static_cast<int>(*it) - 33 < q) {
			res++;
		}
	}
	return res;
}

/* finds a likely match of a string given a library of possible matches
 * arguments:
 * 	string to match
 * 	vector of strings as library
 *	int number of allowed mismatches
 *	*/
int utils::find_likely_match(const string& seq, vector<string>& lookup, int mm) {
	vector<int> mms;

	for (size_t i = 0; i < lookup.size(); ++i) {
//		cout << seq << " " << lookup.at(i);

		if (seq.compare(lookup.at(i)) == 0) { 
//			cout << " identical" << endl;
//			mms.push_back(0);
			return i;
		}
		
		if (seq.length() == lookup.at(i).length()) {
			if (mm == 0) {
				mms.push_back(seq.length());
//				cout << " not identical1" << endl;
			} else {
				mms.push_back(find_diffs(seq, lookup.at(i)));
//				cout << " not identical2" << endl;
			}

		} else {
			string longer, shorter;
			if (seq.length() > lookup.at(i).length()) {
				longer = seq;
				shorter = lookup.at(i);
			} else {
				longer = lookup.at(i);
				shorter = seq; 
			}

			if (longer.find(shorter) != string::npos) {
//				cout << " substr" << endl;
				mms.push_back(0);
			} else { 
//				cout << " no_substr" << endl;
				mms.push_back(seq.length());
			}
		}
	}

	vector<int>::iterator m_it = min_element(mms.begin(), mms.end());
	int min = *m_it;

	int n_min = 0; int min_ind = 0;
	for (size_t i = 0; i < mms.size(); ++i) {
//		cout << mms[i] << " ";
		if (mms.at(i) == min) {
			n_min++;
			min_ind = i;
		}
	}
//	cout << endl;
//	cout << min << " " << n_min << " " << min_ind << endl;
	if ((min > mm) || (n_min > 1)) { return -1; } 
	return min_ind;
}

/* general purpose error reporting function
 * arguments
 * 	string
 *	string
 *	string
 *	*/
void utils::report_error(const string& file, const string& func, const string& error_msg) {
	cerr << "\n" << "ERROR: " << file << "::" << func << ": " << error_msg << endl;
}
