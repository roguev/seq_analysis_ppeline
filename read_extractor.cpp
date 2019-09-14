#include <string>
#include <iostream>
#include <fstream>
#include "fastq_seq.h"
#include "utils.h"
#include <pcrecpp.h>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "read_extractor.h"
#include "gzstream.h"
#include "gzboost.h"
using namespace std;
using namespace utils;

Read_extractor::Read_extractor(
		char* in_fn,
		const string& p_l, 
		const string& p_r, 
		const vector<uint16_t>& mins, 
		const vector<uint16_t>& maxs,
		const vector<string>& ss,
		char* out_fn,
		char* rej_fn) {

	infile = in_fn;
	outfile = out_fn;
	rejected = rej_fn;

	pat_l = p_l;
	pat_r = p_r;
	roi_min = mins;
	roi_max = maxs;
	spacers = ss;
	
	//defaults
	n_threads = 15;
	load_factor = 10000;
	mm_l = true;
	mm_r = true;
	mm_s = true;

	OUTPUT_SEP = "\t";

	with_valid = false;
	with_rejected = false;
	fastq_out = false;

	if (out_fn != NULL) { with_valid = true; }
	if (rej_fn != NULL) { with_rejected = true; }
}

Read_extractor::~Read_extractor() {}

/* setters for various private fields */
void Read_extractor::set_with_valid(bool v) { with_valid = v; }

void Read_extractor::set_with_rejected(bool r) { with_rejected = r; }

void Read_extractor::set_fastq_out(bool f) { fastq_out = f; }

void Read_extractor::set_pat_l(const string& s) { pat_l = s; }
		
void Read_extractor::set_pat_r(const string& s) { pat_r = s; }
		
void Read_extractor::set_roi_mins(vector<uint16_t> i) { roi_min = i; }
		
void Read_extractor::set_roi_min(uint16_t i, size_t p) {
	if (roi_min.size() < p) {
		report_error(__FILE__, __func__, RE_BAD_INDEX);
		exit(REEC_ROI_BAD_INDEX);
	}
	roi_min.at(p) = i;
}

void Read_extractor::set_roi_maxs(vector<uint16_t> i) { roi_max = i; }

void Read_extractor::set_roi_max(uint16_t i, size_t p) {
	if (roi_max.size() < p) {
		report_error(__FILE__, __func__, RE_BAD_INDEX);
		exit(REEC_ROI_BAD_INDEX);
	}
	roi_max.at(p) = i;
}

void Read_extractor::set_spacers(const vector<string>& ss) { spacers = ss; }

void Read_extractor::set_spacer(const string& s, size_t p) {
	if (spacers.size() < p) {
		report_error(__FILE__, __func__, RE_BAD_INDEX);
		exit(REEC_SPACER_BAD_INDEX);
	}
	spacers.at(p) = s;
}

void Read_extractor::set_load_factor(uint32_t i) { load_factor = i; }
		
void Read_extractor::set_n_threads(uint8_t i) { n_threads = i; }

void Read_extractor::set_mm_l(bool m) { mm_l = m; }

void Read_extractor::set_mm_r(bool m) { mm_r = m; }

void Read_extractor::set_mm_s(bool m) { mm_s = m; }

void Read_extractor::set_output_sep(const string& sep) { OUTPUT_SEP = sep; }

/* prijnt current parameters */
void Read_extractor::print_params() {
	cout << "Input:\t";
	if (infile) { cout << infile; } else { cout << "stdin"; }
	cout << endl;

	cout << "Output:\n";
	if (with_valid) {
		cout << "Accept:\t";
		if (outfile) { cout << outfile; } else { cout << "stdout"; } 
		cout << endl;
		cout << "FASTQ output:\t" << fastq_out << endl;
	}
	
	if (with_rejected) {
		cout << "Reject\t";
		if (rejected) { cout << rejected; } else { cout << "stdout"; } 
		cout << endl;
	}
	cout << endl;

	cout << "anchors	sequence	mismatch" << endl;
	cout << "L:	" << pat_l << "\t" << mm_l << endl;
	cout << "R:	" << pat_r << "\t" << mm_r << endl;
	cout << endl;
	cout << "ROI#	Min	Max" << endl;	
	for (size_t i = 0; i < roi_min.size(); ++i) {
		cout << i + 1 << "\t" << roi_min.at(i) << "\t" << roi_max.at(i) << endl;
	}

	cout << endl;
	if (spacers.size() > 0) {
		cout << "spacer	sequence" << endl;	
		for (size_t i = 0; i < spacers.size(); ++i) {
			cout << i + 1 << "\t" << spacers.at(i) << endl;
		}
		cout << endl;
		cout << "spacer mismmach:\t" << mm_s << endl;
		cout << endl;
	}

	cout << "Output sep:\t\"" << OUTPUT_SEP << "\"" << endl;
	cout << "Threads:\t" << +n_threads << endl;
	cout << "Load factor:\t" << load_factor << endl;

}

/* extracts matching seuence reads
 * parameters:
 * 	input stream
 * 	output stream for matched reads
 * 	output stream for rejected reads
 * 	regular expression
 * 	load factor (uint32_t)
 * 	boolean zipped output
 * 	*/
template<class T1>
void Read_extractor::extract_seq_reads(
		T1& in, 
		ofstream& out,
		ofstream& rej,
		pcrecpp::RE& re, 
		uint32_t load_factor,
		bool z_out,
		bool z_rej) {

	vector<Fastq_seq>* seqs = new vector<Fastq_seq>;

	Fastq_seq fq;
	string uid;

	// maximum number of captured groups
	// gives us up to 6 rois
	uint8_t max_groups = 12;
	vector<string> grp(max_groups);

	// hold the current number of groups to be captured
	size_t n_groups = roi_min.size();

	string* out_buffer = new string;
	string* rej_buffer = new string;

	while (1) {
		seqs->clear();
		seqs->reserve(load_factor);
		
		if (with_valid) {
			out_buffer->clear();
			out_buffer->reserve(load_factor*1000);
		}
		
		if (with_rejected) {
			rej_buffer->clear();
			rej_buffer->reserve(load_factor*1000);
		}
		
		// critical
		// read from input into the buffer
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if (in.is_open()) {
				if (!fq.read(in)) { break; }
			} else {
				if (!fq.read(cin)) { break; }
			}
			seqs->push_back(fq);
		}
		mtx.unlock();
		// end critical

		if (seqs->size() == 0) { break; }

		for (size_t n = 0; n < seqs->size(); ++n) {
			Fastq_seq& seq = seqs->at(n);
			
			const string& s = seq.get_seq();
			const string& q = seq.get_qual_str();
			
			uid = seq.get_unique_id();
			bool match = false;

			switch (n_groups) {
				case 1: match =  re.PartialMatch( s, 
								&grp.at(0)); 
					break;
				 	
				case 2: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1));
					break;

				case 3: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2)); 
					break;
			
				case 4: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3)); 
					break;
			
				case 5: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4)); 
					break;
				
				case 6: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5)); 
					break;
				
				case 7: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5), 
								&grp.at(6)); 
					break;
				
				case 8: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5),
								&grp.at(6),
								&grp.at(7)); 
					break;
			
				case 9: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5),
								&grp.at(6),
								&grp.at(7),
								&grp.at(8)); 
					break;
			
			
				case 10: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5), 
								&grp.at(6),
								&grp.at(7),
								&grp.at(8),
								&grp.at(9)); 
					break;
			
				case 11: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5), 
								&grp.at(6),
								&grp.at(7),
								&grp.at(8),
								&grp.at(9),
								&grp.at(10)); 
					break;
			
				case 12: match =  re.PartialMatch( s, 
								&grp.at(0), 
								&grp.at(1), 
								&grp.at(2), 
								&grp.at(3), 
								&grp.at(4), 
								&grp.at(5), 
								&grp.at(6),
								&grp.at(7),
								&grp.at(8),
								&grp.at(9),
								&grp.at(10),
								&grp.at(11)); 
					break;
			}

			if (match) {	// found match
				// output valid reads if needed
				if (with_valid) {
					if (fastq_out) {
					// output fastq file
						*out_buffer += seq.get_seq_id() + "\n";
						for (size_t g = 0; g < n_groups; ++g) {
							*out_buffer += grp.at(g);
						}
						*out_buffer += "\n+\n";
						for (size_t g = 0; g < n_groups; ++g) {
							*out_buffer += 
								q.substr(s.find(grp.at(g)), grp.at(g).length());
						}
						*out_buffer += "\n";
					} else {		
					// output file compatible with downstreram analysis
						*out_buffer += uid + OUTPUT_SEP + seq.get_index();
						for (size_t g = 0; g < n_groups; ++g) {
							*out_buffer +=  
								OUTPUT_SEP +
								grp.at(g) + 
								OUTPUT_SEP +
								q.substr(s.find(grp.at(g)), grp.at(g).length());
						}
						*out_buffer += "\n";
					}
				}
			} else {	// no match
				// output rejected reads if needed
				if (with_rejected) { *rej_buffer += seq.to_string(); }
			}
		}

		if (z_out) {
			if (with_valid)		{ *out_buffer = compress_string(*out_buffer); }
			if (with_rejected)	{ *rej_buffer = compress_string(*rej_buffer); }
		}

		// critical
		// write into output streams
		mtx.lock();

		if (with_valid) {
			if (out.is_open()) {
				out << *out_buffer;
			} else {
				cout << *out_buffer;
			}
		}
		
		if (with_rejected) {
			if (rej.is_open()) {	
				rej << *rej_buffer;
			} else {
				cout << *out_buffer;
			}
		}

		mtx.unlock();
		// end critical
	}
	delete(out_buffer);
	delete(rej_buffer);
}

/* main function to call from a program
 * parameters
 * 	boolean zipped input
 * 	boolean zipped output
 * 	*/
void Read_extractor::extract(bool z_in, bool z_out, bool z_rej) {
	boost::thread_group tgroup;

	ifstream i1;
	igzstream z1;
	
	ofstream out;
	ofstream rej;
	
	if (with_valid && outfile) {
		attach_stream<ofstream>(outfile, out, std::ios_base::out | std::ios_base::binary); 
	}
	
	if (with_rejected && rejected) {
		attach_stream<ofstream>(rejected, rej, std::ios_base::out | std::ios_base::binary);
	}

	re_str = gen_regex_string(
			pat_l, 
			pat_r, 
			mm_l, 
			mm_r, 
			mm_s, 
			roi_min, 
			roi_max, 
			spacers);

	pcrecpp::RE re(re_str);

	// everything uncompressed
	if (!z_in) {	
		if (infile) { attach_stream<ifstream>(infile, i1, std::ios_base::in); }
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Read_extractor::extract_seq_reads<ifstream>, 
						this, 
						boost::ref(i1),
						boost::ref(out),
						boost::ref(rej),
						boost::ref(re), 
						load_factor,
						z_out,
						z_rej
						)
					);
		}
		tgroup.join_all();
		if (i1.is_open()) { i1.close(); }
	}

	// compressed input and uncompressed output
	if (z_in) {	
		if (infile) { attach_stream<igzstream>(infile, z1, std::ios_base::in); }
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Read_extractor::extract_seq_reads<igzstream>, 
						this, 
						boost::ref(z1),
						boost::ref(out),
						boost::ref(rej),
						boost::ref(re), 
						load_factor,
						z_out,
						z_rej
						)
					);
		}
		tgroup.join_all();
		if (z1.is_open()) { z1.close(); }
	}

	if (out.is_open()) { out.close(); }
	if (rej.is_open()) { rej.close(); }
}
