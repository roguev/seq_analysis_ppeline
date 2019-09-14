#include <string>
#include <iostream>
#include <fstream>
#include "fastq_seq.h"
#include "utils.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "seq_stats.h"
#include "gzstream.h"
#include "matrix.h"
using namespace std;
using namespace utils;

const string SS_BAD_READ_LENGTH = 	"Invalid read length";

enum SS_ERRORS {
	SSEC_BAD_READ_LENGTH = 	1
};

/* constructor */
Seq_stats::Seq_stats(char* in_fn, char* out_fn, uint16_t r_len) {
	infile = in_fn;
	outfile = out_fn;

	read_length = r_len;
	init_containers();

	// defaults
	OUTPUT_SEP = "\t";
	INPUT_SEP = "\t";
	load_factor = 25000;
	n_threads = 4;

	total_seqs = 0;
	valid_seqs = 0;
	raw_input = false;
}

/* setters for various parameters */
// output delimiter
void Seq_stats::set_output_sep(const string sep) { OUTPUT_SEP = sep; }

// input delimiter
void Seq_stats::set_input_sep(const string sep) { INPUT_SEP = sep; }

// load factor
void Seq_stats::set_load_factor(uint32_t i) { load_factor = i; }

// number of threads
void Seq_stats::set_n_threads(uint8_t i) { n_threads = i; }

// read lenght for this sequencing run
void Seq_stats::set_read_length(uint16_t i) { read_length = i; }

// use red_extractor outout or FASTQ sequence
void Seq_stats::set_raw_input(bool r) { raw_input = r; }

/* destructor */
Seq_stats::~Seq_stats() {}

/* prints run parameters */
void Seq_stats::print_params() {
	cout << "Input\t";
	if (infile) { cout << infile; } else { cout << "cin"; }
	
	cout << "Input format\t";
	if (raw_input) { cout << "Raw"; } else { cout << "FASTQ"; }
	cout << endl;

	cout << "Output:\t";
	if (outfile) { cout << outfile; } else { cout << "cout"; }
	cout << endl;

	cout << endl;
	cout << "Read length:\t" << read_length << endl;
	cout << "Input sep:\t\"" << INPUT_SEP << "\"" << endl;
	cout << "Output sep:\t\"" << OUTPUT_SEP << "\"" << endl;
	cout << endl;

	cout << "Threads:\t" << +n_threads << endl;
	cout << "Load factor:\t" << load_factor << endl;
}

/* initializes data storage containers
 * with zoros */
void Seq_stats::init_containers() {
	vector<uint32_t> x(read_length, 0);
	A = x;
	C = x;
	G = x;
	T = x;
	N = x;
	pos_counts = x;

	vector<uint64_t> y(read_length, 0);
	Q = y;

	vector<uint32_t> z(read_length+2,0);
	L = z;
}

/* converts a raw sequence record from the read_extractor module
 * to a FASTQ sequence
 * parameters
 * 	string$ raw record
 * 	fastq_seq& fastq sequence
 * 	*/

void Seq_stats::raw2fastq(string& raw, Fastq_seq& fq) {
	// split raw record using a delimiter
	vector<string> chunks = split_string(raw, INPUT_SEP);
	
	// form the sequence id
	string id = chunks.at(0) + "_" + chunks.at(1);
	
	string seq;
	string qual;
	// for the sequence and quality strings
	for (size_t i = 2; i < chunks.size()-1;++i) {
		seq += chunks.at(i);
		qual += chunks.at(i+1);
	}

	// set fields in the resulting fastq sequence
	fq.set_seq_id(id);
	fq.set_seq(seq);
	fq.set_qual_str(qual);
}

/* processed a sequence file
 * parameters
 * 	inout stream
 * 	*/
template <class T1>
void Seq_stats::process_seqs(T1& in) {

	vector<Fastq_seq>* seqs = new vector<Fastq_seq>;
	Fastq_seq fq;
	string raw;
	
	// valid reads count
	uint32_t vs = 0;

	// temporary vectors to hold results
	vector<uint32_t> tmpA(read_length, 0);
	vector<uint32_t> tmpT(read_length, 0);
	vector<uint32_t> tmpG(read_length, 0);
	vector<uint32_t> tmpC(read_length, 0);
	vector<uint32_t> tmpN(read_length, 0);
	vector<uint32_t> tmpPC(read_length, 0);
	vector<uint64_t> tmpQ(read_length, 0);
	
	// adding two additional bins - for empty seqs and for seqs longer than the rad length
	vector<uint32_t> tmpL(read_length + 2, 0);

	while (1) {
		seqs->clear();
		seqs->reserve(load_factor);

		// critical
		// read from input into the buffer
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if (in.is_open()) {
				if (raw_input) {
					// hanldle raw input, file
					if (!getline(in, raw)) { break; } else { raw2fastq(raw,fq); } 
				} else {
					// handle fastq input, file
					if (!fq.read(in)) { break; }
				}
				
			} else {
				if (raw_input) {
					// handle raw input, cin
					if (!getline(cin, raw)) { break; } else { raw2fastq(raw,fq); }
				} else {
					// handle fastq, cin
					if (!fq.read(cin)) { break; }
				}
			}
			seqs->push_back(fq);
		}
		total_seqs += seqs->size();
		mtx.unlock();
		// end critical
		
		if (seqs->size() == 0) { break; }

		for (size_t i = 0; i < seqs->size(); ++i) {
			fq = seqs->at(i);
			const string s = fq.get_seq();
			const string q = fq.get_qual_str();
			size_t seq_l = s.length();

			// add data to the size distriburion
			if (seq_l > read_length) {
				// last bin
				tmpL.back()++;
				continue;
			} else { 
				tmpL.at(seq_l)++; 
			}

			// remove empty reads
			if (seq_l == 0) { 
				// first bin
				tmpL.front()++;
				continue; 
			} 
			
			// increase the valid reads counter
			vs++;

			// add to the stats vectors
			for (size_t j = 0; j < seq_l; ++j) {
				tmpQ.at(j) += static_cast<uint64_t>(q.at(j)) - 33;
				tmpPC.at(j)++;

				switch(s.at(j)) {
					case('A'): case('a'): tmpA.at(j)++; break;
					case('T'): case('t'): tmpT.at(j)++; break;
					case('G'): case('g'): tmpG.at(j)++; break;
					case('C'): case('c'): tmpC.at(j)++; break;
					case('N'): case('n'): tmpN.at(j)++; break;
				}
			}
		}
	}

	// critical
	// add data to main containers
	mtx.lock();
	valid_seqs += vs;
	
	// sequence composition stats
	for (size_t i = 0; i < read_length; ++i) {
		A.at(i) += tmpA.at(i);
		T.at(i) += tmpT.at(i);
		G.at(i) += tmpG.at(i);
		C.at(i) += tmpC.at(i);
		N.at(i) += tmpN.at(i);
		Q.at(i) += tmpQ.at(i);
		pos_counts.at(i) += tmpPC.at(i);
	}

	// length distribution
	for (size_t i = 0; i < read_length + 2; ++i) {
		L.at(i) += tmpL.at(i);
	}
	mtx.unlock();
	// end critical

	// cleanup
	delete(seqs);
}
			
/* the main function of the class. Call this from within your program
 * and before any of the stats print methods
 * parameters
 * 	bool showing if the onput is compressed or now
 * 	*/
void Seq_stats::collect_stats(bool z_in) {
	boost::thread_group tgroup;
	
	ifstream i1;
	igzstream z1;

	// uncompressed input
	if (!z_in) {
		// open file
		if (infile) { attach_stream<ifstream>(infile, i1, std::ios_base::in); }

		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Seq_stats::process_seqs<ifstream>, 
						this, 
						boost::ref(i1)
						)
					);
		}
		tgroup.join_all();

		// close file
		if (i1.is_open()) { i1.close(); }
	}

	// compressed input
	if (z_in) {	
		// open file
		if (infile) { attach_stream<igzstream>(infile, z1, std::ios_base::in); }
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Seq_stats::process_seqs<igzstream>, 
						this, 
						boost::ref(z1)
						)
					);
		}
		tgroup.join_all();
		
		// close file
		if (z1.is_open()) { z1.close(); }
	}
}

/* scales the values given a denominator
 * parameters
 * 	vector
 * 	numeric denominator
 * 	*/
template<class T1, class T2, class T3>
vector<T3> Seq_stats::scale_vector(T1& v, T2& denom) {
	vector<T3> res;
	long double val;
	for (size_t i = 0; i < v.size(); ++i) {
		val = static_cast<long double>(v.at(i))/static_cast<long double>(denom);
		res.push_back(static_cast<T3>(val));
	}
	return res;
}

/* outputs stats to a output stream
 * parameters
 * 	int mode
 * 	*/
void Seq_stats::output_stats(SS_OUTPUT_MODE mode) {
	string output;

	// cast mode back to uint8_t
	uint8_t m = static_cast<uint8_t>(mode);

	// testing for different bits being set
	// allows for combining different modes with a '|' operator
	if ((m & 1) != 0) { output += get_counts(); }			// bit 1
	if ((m & 2) != 0) { output += get_nt_frequency_stats(); }	// bit 2
	if ((m & 4) != 0) { output += get_nt_quality_stats(); }		// bit 3
	if ((m & 8) != 0) { output += get_len_distribution_stats(); }	// bit 4
	if ((m & 16) != 0){ output += get_full_stats(); }		// bit 5

	// cout
	if (!outfile) { 
		cout << "MODE" << OUTPUT_SEP << static_cast<uint16_t>(mode) << "\n";
		cout << "READ_LENGTH" << OUTPUT_SEP << read_length << "\n";
		cout << output;
	} else {
		// file
		ofstream out;
		attach_stream<ofstream>(outfile, out, std::ios_base::out);
		out << "MODE" << OUTPUT_SEP << static_cast<uint16_t>(mode) << "\n";
		out << "READ_LENGTH" << OUTPUT_SEP << read_length << "\n";
		out << output;
		out.close();
	}
}

/* various printing routines */
string Seq_stats::get_nt_frequency_stats() {
	vector<double> bins;
	string res;

	for (size_t i = 1; i <= read_length; ++i) {
		bins.push_back(static_cast<double>(i));
	}

	// create a matrix with bins rows and one column
	// containing the bin IDs
	Matrix<double> m(bins, bins.size(), 1);

	vector<double> sA(read_length, 0);
	vector<double> sT(read_length, 0);
	vector<double> sG(read_length, 0);
	vector<double> sC(read_length, 0);
	vector<double> sN(read_length, 0);

	for (size_t i = 0; i < read_length; ++i) {
		sA.at(i) = static_cast<double>(A.at(i))/static_cast<double>(pos_counts.at(i));
		sT.at(i) = static_cast<double>(T.at(i))/static_cast<double>(pos_counts.at(i));
		sG.at(i) = static_cast<double>(G.at(i))/static_cast<double>(pos_counts.at(i));
		sC.at(i) = static_cast<double>(C.at(i))/static_cast<double>(pos_counts.at(i));
		sN.at(i) = static_cast<double>(N.at(i))/static_cast<double>(pos_counts.at(i));
	}

	// add a column to the matrix
	// we endup with a matrix in the format
	// bin_ID %A %T %G %C %N
	m.add_col(m.c, sA);
	m.add_col(m.c, sT);
	m.add_col(m.c, sG);
	m.add_col(m.c, sC);
	m.add_col(m.c, sN);

	for (size_t i = 0; i < m.r; ++i) {
		res += "NT_FREQ" + OUTPUT_SEP + m.row_to_string(i, OUTPUT_SEP);
	}

	return res;
}

string Seq_stats::get_nt_quality_stats() {
	vector<double>bins;
	string res;

	for (size_t i = 1; i <= read_length; ++i) {
		bins.push_back(static_cast<double>(i));
	}

	// create a matrix with bins rows and 1 column
	// containing the bin IDs
	Matrix<double> m(bins, bins.size(), 1);

	vector<double> sQ(read_length, 0);
	for (size_t i = 0; i < read_length; ++i) { sQ.at(i) = static_cast<double>(Q.at(i))/static_cast<double>(pos_counts.at(i)); }

	m.add_col(m.c, sQ);

	for (size_t i = 0; i < m.r; ++i) {
		res += "QUAL" + OUTPUT_SEP + m.row_to_string(i, OUTPUT_SEP); 
	}
	
	return res;
}

string Seq_stats::get_len_distribution_stats() {
	vector<double>bins;
	string res;

	for (size_t i = 0; i < read_length+2; ++i) {
		bins.push_back(static_cast<double>(i));
	}

	// create a matrix with bins rows and 1 column
	// containing the bin IDs
	Matrix<double> m(bins, bins.size(), 1);

	vector<double> sL = scale_vector<vector<uint32_t>, uint32_t, double>(L, valid_seqs);
	m.add_col(m.c, sL);

	for (size_t i = 0; i < m.r; ++i) {
		res += "LEN" + OUTPUT_SEP + m.row_to_string(i, OUTPUT_SEP);
	}
	
	return res;
}

string Seq_stats::get_counts() {
	string tmp;
	tmp += "TOTAL_READS" + OUTPUT_SEP + to_string(static_cast<long long int>(total_seqs)) + "\n";
	tmp += "VALID_READS" + OUTPUT_SEP + to_string(static_cast<long long int>(valid_seqs)) + "\n";

	return tmp;

}

string Seq_stats::get_full_stats() {
	string tmp;
	tmp += get_counts();	
	tmp += get_nt_frequency_stats();
	tmp += get_nt_quality_stats();
	tmp += get_len_distribution_stats();

	return tmp;
}
