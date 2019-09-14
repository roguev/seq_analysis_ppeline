#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include "fastq_seq.h"
#include "utils.h"
#include <stdlib.h>
using namespace std;
/* The Fastq_seq object encapsulates a fastq sequence record as output by NGS instruments */

// cosntructors and destructors
Fastq_seq::Fastq_seq() {
	// reserve 200 characters, should be adequate for most applications
	seq_id.reserve(200);
	seq.reserve(200);
	q_score_id.reserve(1);
	qual_str.reserve(200);
}

Fastq_seq::~Fastq_seq() {}

// getters
.// ID
const string& Fastq_seq::get_seq_id() const { return seq_id; }

// sequence string
const string& Fastq_seq::get_seq() const { return seq; }

// quality string
const string& Fastq_seq::get_qual_str() const { return qual_str; }

// setters
// ID
void Fastq_seq::set_seq_id(const string& s) { seq_id = s; }

// sequence string
void Fastq_seq::set_seq(const string& s) { seq = s; }

// quality string
void Fastq_seq::set_qual_str(const string& s) { qual_str = s; }

// io
// reads fastq sequence from a stream
bool Fastq_seq::read(istream& in) {
	// load the first field
	while (getline(in,seq_id)) {
		// found an entry point
		if (seq_id.at(0) == '@') { 
			// read three more lines and populate fields
			if (!getline(in,seq)) { return false; }
			if (!getline(in,q_score_id)) { return false; }
			if (!getline(in,qual_str)) { return false; }

			// all good
			return true;
		}
	}
	return false;
}

// returns a string representation of a Fastq_seq using newline as a field delimiter
string Fastq_seq::to_string() const {
	string nl("\n");
	return seq_id + nl + seq + nl + q_score_id + nl + qual_str + nl;
}

// writes fastq sequence to a stream
void Fastq_seq::write(ostream& out) const {
	out << seq_id << endl;
	out << seq << endl;
	out << q_score_id << endl;
	out << qual_str << endl;
}

// tokanizes the sequence header and writes it into a SEQ_HEADER structure
// for easy access downstream
SEQ_HEADER Fastq_seq::parse_seq_id() const {
	SEQ_HEADER res;

	// tokenize the sequence ID using ':' as delimiter
	vector<string> chunks = utils::split_string(seq_id, string(":"));

	// check if it is a valid ID
	if (chunks.size() != 10) {
		utils::report_error(__FILE__, __func__, "Invalid sequence header: wrong number of elements");
		exit(10);
	}
	
	// populate the SEQ_HEADER srtucture
	res.instrument = chunks.at(0);
	res.run_number = static_cast<uint16_t>(atoi(chunks.at(1).c_str()));
	res.flowcell_id = chunks.at(2);
	res.lane = static_cast<uint8_t>(atoi(chunks.at(3).c_str()));
	res.tile = static_cast<uint16_t>(atoi(chunks.at(4).c_str()));
	res.x = static_cast<uint32_t>(atoi(chunks.at(5).c_str()));
	vector<string> t = utils::split_string(chunks.at(6), " ");

	if (t.size() != 2) {
		utils::report_error(__FILE__, __func__, "Invalid sequence header: wrong y_coord-read combo");
		exit(11);
	}

	res.y = static_cast<uint32_t>(atoi(t.at(0).c_str()));
	res.read_number = static_cast<uint8_t>(atoi(t.at(1).c_str()));
	
	if (chunks.at(7).compare("Y")) { res.filtered = true; }
	else { res.filtered = false; }

	res.control_number = static_cast<uint16_t>(atoi(chunks.at(8).c_str()));
	res.index = chunks.at(9);
	return res;
}

// print to screen
void Fastq_seq::print() const {
	write(std::cout);
}

// trim the sequence header by removing select parts of it
string Fastq_seq::trim_seq_id(vector<int>& trim) const {
	vector<string> parts = utils::split_string(seq_id, string(":"));
	string result;
	
	for (uint8_t i = 0; i < parts.size(); ++i) {
		if (find(trim.begin(), trim.end(), i) == trim.end()) {
			result += parts[i];
			result.push_back(':');
		}
	}
	result.erase(result.size() - 1);
	return result;
}

// returns the index part of the sequence id
string Fastq_seq::get_index() const {
	return seq_id.substr(seq_id.find_last_of(":")+1);
}

// generates a unique sequence id
string Fastq_seq::get_unique_id() const {
	return seq_id.substr(0, seq_id.find_first_of(" "));
}
