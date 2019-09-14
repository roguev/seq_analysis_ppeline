#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "utils.h"
#include "gzstream.h"
#include "read_counter.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>
using namespace std;
using namespace utils;

/* constructor
 * sets up stuff, and initialize hashes on the hreap
 * arguments:
 * 	ID map file
 * 	output file
 * 	sample map file
 * 	vector of mapping files
 * 	vector of rt flags
 * 	reverse complement index (true or false)
 * 	minimum base quality
 */
Read_counter::Read_counter(
		char* in,
		char* out,
		char* smap,
		const vector<char*>& ms
		) {

	infile = in;
	outfile = out;
	sample_map = smap;
	read_maps = ms;

	set_defaults();
	init_hashes();

	// populate the mismatch and rcs vectors
	for (size_t i = 0; i < read_maps.size(); ++i) {
		mms.push_back(1);
		rcs.push_back(false);
	}
}

/* sets some default values
 * takes no arguments
 * */
void Read_counter::set_defaults() {
	index_mm = 		1;
	idxrc = 		false;
	
	max_lq_bases = 		5;
	min_qual = 		20;

	n_threads = 		15;

	collapser_bite_size = 	100000;
	counter_bite_size = 	1000;

	READ_UNKNOWN_TAG = 	"unknown";
	READ_Q_FAIL_TAG = 	"Q_FAIL";
	REC_SEPARATOR = 	":";
	IDX_UNDEF_TAG = 	"undef";
	
	HASH_SEP = string(1,128);	// non-printable
	INPUT_SEP = 		"\t";
	OUTPUT_SEP = 		"\t";
	MAP_SEP = 		"\t";

	with_fails = 		true;
	with_unknowns = 	true;
	with_undefs = 		true;

	collapse_q_fails = 	true;

	with_raw_stats = 	false;
}

/* prints current parameters
 * takes no argumenst
 * */
void Read_counter::print_params() {
	cout << "Input file:\t";
	if (infile) { cout << infile; } else { cout << "stdin"; }
	cout << endl;

	cout << "Output file:\t"; 
	if (outfile) { cout << outfile; } else { cout << "stdout"; } 
	cout << endl;

	cout << "Sample map:\t" << sample_map << endl;
	
	if (with_raw_stats && stats_file) {
		cout << "Stats file:\t" << stats_file << endl;
	}

	for (size_t i = 0; i < read_maps.size(); ++i) {
		cout << "map_" << i+1 << ":\t" << read_maps.at(i) << endl;
	}

	for (size_t i = 0; i < rcs.size(); ++i) {
		cout << "rc_" << i+1 << ":\t" << rcs.at(i) << endl;
	}

	for (size_t i = 0; i < mms.size(); ++i) {
		cout << "mm_" << i+1 << ":\t" << +mms.at(i) << endl;
	}

	cout << "rc index:	" << idxrc << endl;
	cout << "Index mm:	" << +index_mm << endl;
	
	cout << "with_undefs:	" << with_undefs << endl;
	cout << "with_fails:	" << with_fails << endl;
	cout << "with_unknowns:	" << with_unknowns << endl;

	cout << "coll_q_fails	" << collapse_q_fails << endl;

	cout << "with_raw_stats	" << with_raw_stats << endl;

	cout << "rec separator:	\"" << REC_SEPARATOR << "\"" << endl;
	cout << "input sep:	\"" << INPUT_SEP << "\"" << endl;
	cout << "output_sep:	\"" << OUTPUT_SEP << "\"" << endl;
	cout << "map_sep:	\"" << MAP_SEP << "\"" << endl;

	cout << "fail tag:	" << READ_Q_FAIL_TAG << endl;
	cout << "unkn tag:	" << READ_UNKNOWN_TAG << endl;

	cout << "undef tag:	" << IDX_UNDEF_TAG << endl;
	cout << "Min quality:	" << +min_qual << endl;
	cout << "Max lq_bases:	" << +max_lq_bases << endl;

	cout << "Collapse bite:	" << collapser_bite_size << endl;
	cout << "Counter bite	" << counter_bite_size << endl;
	cout << "N threads:	" << +n_threads << endl;
}

/* initialized internal hashes on the heap
 * takes no arguments
 * */
void Read_counter::init_hashes() {
	counts_hash	= new umsi;
	translated	= new umsi;

	stats_idx       = new umsi;
	for (size_t i = 0; i < read_maps.size(); ++i) {
		stats_r.push_back(new umsi);
	}
}

/* destroy stuff initialize of the heap from the constructor 
 * takes no arguments
 * */
Read_counter::~Read_counter() {
	delete(counts_hash);
	delete(translated);

	for (size_t i = 0; i < stats_r.size(); ++i) {
		delete(stats_r.at(i));
	}
}

/* mismatch setters */
void Read_counter::set_mm(uint8_t n, uint8_t p) {
	if (p > mms.size() - 1) {
		report_error(__FILE__, __func__, RC_BAD_INDEX);
		exit(RCEC_BAD_INDEX);
	}
	mms.at(p) = n;
}

void Read_counter::set_mms(const vector<uint8_t>& mm) { mms = mm; }

void Read_counter::set_index_mm(uint8_t n) { index_mm = n; }

/* setters for different tags */
void Read_counter::set_read_unknown_tag(const string& tag) { READ_UNKNOWN_TAG = tag; }

void Read_counter::set_read_q_fail_tag(const string& tag) { READ_Q_FAIL_TAG = tag; }

void Read_counter::set_idx_undef_tag(const string& tag) { IDX_UNDEF_TAG = tag; }

void Read_counter::set_record_separator(const string& s) { REC_SEPARATOR = s; }

void Read_counter::set_input_sep(const string& s) { INPUT_SEP = s; }

void Read_counter::set_output_sep(const string& s) { OUTPUT_SEP = s; }

void Read_counter::set_map_sep(const string& s) { MAP_SEP = s; }

/* setter for number of threads, default is 10, see constructor */
void Read_counter::set_n_threads(uint8_t n) { n_threads = n; }

/* performance tuming setters */
void Read_counter::set_collapser_bite_size(uint32_t n) { collapser_bite_size = n; }

void Read_counter::set_counter_bite_size(uint32_t n) { counter_bite_size = n; }

/* quality filter setters */
void Read_counter::set_min_qual(uint8_t n) { min_qual = n; }

void Read_counter::set_max_lq_bases(uint8_t n) { max_lq_bases = n; }

/* filenames */
void Read_counter::set_input(char* f) { infile = f; }

void Read_counter::set_output(char* f) { outfile = f; }

void Read_counter::set_stats(char* f) { stats_file = f; }

void Read_counter::add_map(char* map) {
	if (map) { read_maps.push_back(map); }
}

void Read_counter::set_smap(char* f) { sample_map = f; }

/* revcom setters */
void Read_counter::set_rc(bool i, uint8_t n) {
	if (n > rcs.size() - 1) {
		report_error(__FILE__, __func__, RC_BAD_INDEX);
		exit(RCEC_BAD_INDEX);
	}
	rcs.at(n) = i;
}

void Read_counter::set_rcs(const vector<bool>& rc) { rcs = rc; }

void Read_counter::set_idxrc(bool i) { idxrc = i; }

/*output setters */
void Read_counter::set_with_fails(bool i) { with_fails = i; }

void Read_counter::set_with_unknowns(bool i) { with_unknowns = i; }

void Read_counter::set_with_undefs(bool i) { with_undefs = i; }

void Read_counter::set_with_raw_stats(bool i) { with_raw_stats = i; }

void Read_counter::set_collapse_q_fails(bool i) { collapse_q_fails = i; }

/* used to reduce the complexity of the data by lumping together identical reads
 * drastically improves performance */
template<class T>
void Read_counter::collapse_reads(
		T& in,
		umss& sample_map) {

	// for processing the input
	string line;
	vector<string>* in_buffer = new vector<string>;
	vector<string> chunks;
	string key;

	// temporary hashes
	umsi* temp_counts = new umsi;;
	umsi* temp_stats = new umsi;
	umss* mapped = new umss;

	vector<string> lookup;
	for (umss_it it = sample_map.begin(); it != sample_map.end(); ++it) {
		lookup.push_back(it->first);
	}

	while (1) {
		in_buffer->clear();
		in_buffer->reserve(collapser_bite_size);

		temp_counts->clear();
		temp_stats->clear();

		// critical
		// lock mutex
		// read from input and populate the in_buffer
		// reading in chunks of 10000 records
		collapse_mtx.lock();
		for (uint32_t i = 0; i < collapser_bite_size; ++i) {
			if (in.is_open()) {
				if (!getline(in, line)) { break; } 
			} else {
				if (!getline(cin, line)) { break; }
			}
			// cout << line;
			in_buffer->push_back(line);
		}
		collapse_mtx.unlock();
		// end critical

		// exit loop
		if (in_buffer->size() == 0) { break; }
		
		// iterate over the in_buffer
		for (size_t j = 0; j < in_buffer->size(); ++j) {
			// set sample as undefined by default
			string sample = IDX_UNDEF_TAG;
			chunks = split_string(in_buffer->at(j), INPUT_SEP);

			sample = match_with_helper(
						chunks.at(1),
						lookup,
						sample_map,
						*mapped,
						index_mm,
						IDX_UNDEF_TAG); 

			if (with_raw_stats) {
				(*temp_stats)[sample + OUTPUT_SEP + chunks.at(1)]++;
			}
			
			// check if the record is ok
			size_t rec_sz = chunks.size() - 1;
			uint8_t nr = (uint8_t) (rec_sz/3);
			if ((nr < 1) || (rec_sz % 3 > 0)) { 
				report_error(__FILE__, __func__, RC_CORRUPT_RECORD);
				report_error(__FILE__, __func__,  in_buffer->at(j));
				exit(RCEC_COLLAPSER_CORRUPT_RECORD); 
			}
			
			// always do read1, we assume at least 1 read and check sequence quality
			vector<string> seqs;
			string k;
			for (uint8_t r = 0; r < nr; ++r) {
				k = (seq_qual(chunks.at(3*r+3), min_qual) <= max_lq_bases) ? chunks.at(3*r+2) : READ_Q_FAIL_TAG;
				seqs.push_back(k);
			}
			
			// collapse the quality failed reads?
			bool failed = false;
			if (collapse_q_fails) {
			// if any of the reads are poor quality tag the whole thing as bad
				for (size_t i = 0; i < seqs.size(); ++i) {
					if (seqs.at(i).compare(READ_Q_FAIL_TAG) == 0) {
						failed = true;
					}
				}
				
				if (failed) {
					for (size_t i = 0; i < seqs.size(); ++i) {
						seqs.at(i) = READ_Q_FAIL_TAG;
					}
				}
			}
			
			string key;
			for (size_t i = 0; i < seqs.size(); ++i) {
				key += seqs.at(i) + HASH_SEP;
			}
			key += sample;
			seqs.clear();
			// add to temporary counts hash
			(*temp_counts)[key]++;
		}
		
		// critical
		// lock mutex and add the temporary counts hash to the main counts hash
		collapse_mtx.lock();
		for (umsi_it it = temp_counts->begin(); it != temp_counts->end(); ++it) {
			(*counts_hash)[it->first] += it->second;
		}

		// write stats if needed
		if (with_raw_stats) {
			for (umsi_it it = temp_stats->begin(); it != temp_stats->end(); ++it) {
				 (*stats_idx)[it->first] += it->second;
			}
		}
		collapse_mtx.unlock();
		// end critical
	}
	// celanup
	delete(temp_counts);
	delete(mapped);
	delete(in_buffer);
	delete(temp_stats);
}

/* main read counter function
 * argumenst:
 *	output final hash with human readable IDs and counts
 *	hash mapping sequences to human readables for R1
 *	hash mapping sequences to human readables for R2
 *	*/

void Read_counter::count_reads(vector<umss>& maps) { 
	string line;
	vector<string>* in_buffer = new vector<string>;
	vector<string> chunks;
	vector<string> pair;

	static umsi_it main_it = counts_hash->begin();

	// temporary hashes
	umsi* temp_counts = new umsi;
	umsi* buffer = new umsi;

	// used for find_likely_match function
	vector< vector<string> > lookup;

	// temporary hases
	vector<umss*> mapped;
	for (size_t i = 0; i < maps.size(); ++i) {
		vector<string> l;
		for (umss_it it = maps.at(i).begin(); it != maps.at(i).end(); ++it) {
			l.push_back(it->first);
		}
		lookup.push_back(l);
		l.clear();

		mapped.push_back(new umss);
	}

	vector<umsi*> temp_stats;
	for (size_t i = 0; i < stats_r.size(); ++i) {
		temp_stats.push_back(new umsi);
	}

	// check if we reached the end of the count hash
	while (main_it != counts_hash->end()) {
		in_buffer->clear();
		in_buffer->reserve(counter_bite_size);
		temp_counts->clear();
		buffer->clear();

		for (size_t i = 0; i < temp_stats.size(); ++i) {
			temp_stats.at(i)->clear();
		}

		// critical
		// lock mutex and read from counts hash and populate the buffer hash
		counter_mtx.lock();
		for (uint32_t j = 0; j < counter_bite_size; ++j) {
			if (main_it == counts_hash->end()) { break; }
			(*buffer)[main_it->first] = main_it->second;
			++main_it;
		}
		counter_mtx.unlock();
		// end critical
		
		// iterate over the buffer hash
		for (umsi_it br_i = buffer->begin(); br_i != buffer->end(); ++br_i) {
			string k;
			string p_str = br_i->first;
			pair = split_string(p_str, HASH_SEP);
			string key = pair.back() + HASH_SEP;

			int num_reads = pair.size() - 1;

			if (num_reads == 0) {
				report_error(__FILE__, __func__, RC_CORRUPT_RECORD);
				report_error(__FILE__, __func__, p_str);
				exit(RCEC_COUNTER_CORRUPT_RECORD);
			}

			if (num_reads > maps.size()) {
				report_error(__FILE__, __func__, RC_BAD_MAPPING);
				report_error(__FILE__, __func__, p_str);
				exit(RCEC_COUNTER_BAD_MAPPING);
			}

			// always assume a single read
			// try to figure out human readable id
			for (size_t r = 0; r < num_reads; ++r) {		
				k = READ_UNKNOWN_TAG;
				if (pair.at(r).compare(READ_Q_FAIL_TAG) == 0) { k = READ_Q_FAIL_TAG; } 
				else { k = match_with_helper(
						pair.at(r),
						lookup.at(r),
						maps.at(r),
						*mapped.at(r),
						mms.at(r),
						READ_UNKNOWN_TAG); }
			
				if (with_raw_stats) {
					(*temp_stats.at(r))[k + OUTPUT_SEP + pair.at(r)] += br_i->second;
				}

				if (r == num_reads - 1) {
					key += k;
				} else {
					key += k + REC_SEPARATOR;
				}
			}

			// add to the temporary counts hash
			(*temp_counts)[key] += br_i->second;
		}
		
		// critical
		// lock mutex and update hashes
		counter_mtx.lock();
		for (umsi_it it = temp_counts->begin(); it != temp_counts->end(); ++it) {
			// filter depending on output filtering options
			if ((it->first.find(READ_Q_FAIL_TAG) != string::npos) && (!with_fails)) { continue; }
			if ((it->first.find(READ_UNKNOWN_TAG) != string::npos) && (!with_unknowns)) { continue; }
			if ((it->first.find(IDX_UNDEF_TAG) != string::npos) && (!with_undefs)) { continue; }
			(*translated)[it->first] += it->second;
		}

		if (with_raw_stats) {
			for (size_t i = 0; i < temp_stats.size(); ++i) {
				for (umsi_it it = temp_stats.at(i)->begin(); it != temp_stats.at(i)->end(); ++it) {
					(*stats_r.at(i))[it->first] += it->second;
				}
			}
		}
		counter_mtx.unlock();
		// end critical
	}
	// cleanup
	delete(in_buffer);
	delete(temp_counts);
	
	for (size_t i = 0; i < mapped.size(); ++i) {
		delete(mapped.at(i));
	}
	
	for (size_t i = 0; i < temp_stats.size(); ++i) {
		delete(temp_stats.at(i));
	}

	delete(buffer);
}

/* attempts to match sequence to a human-redable ID using a helper hash of
 * already known mappings
 * arguments:
 * 	sequence to be matched
 * 	vector of luukup sequences
 *	hash mapping sequences to human readables
 *	temporary mapped hash
 *	allowed mismatches
 *	*/
string Read_counter::match_with_helper(
		string& seq, 
		vector<string>& lookup, 
		umss& mapping, 
		umss& tmm,
		uint8_t mm,
		string& tag) {

	// if not found n the mapped hash try to figure it ouy
	if (tmm.find(seq) == tmm.end()) {
		int m_idx = find_likely_match(seq, lookup, mm);
		if (m_idx >= 0) {
			// new mapping discovered, add it to the temporary mapped hash
			tmm[seq] = mapping[lookup.at(m_idx)];
			return tmm[seq];
		} else {
			// cannot figure it out
			return tag;
		}
	} else {
		// if found in the mapped hash
		return tmm[seq];		
	}
}

/* a helper function for populating mapping hashes
 * arguments:
 * 	fikename
 * 	revcom flag
 * 	*/
umss Read_counter::load_mapping(char* fn, bool rc) {
	string line;
	boost::unordered::unordered_map<string, string> res;
	vector<string> parts;

	ifstream in;
	attach_stream<ifstream>(fn, in, std::ifstream::in);
	
	// read the file into a mapping hash
	while(in.good()) {
		if (getline(in, line)) {
			parts = split_string(line, MAP_SEP);
			if (parts.size() < 2) {
				report_error(__FILE__, __func__, RC_CORRUPT_MAP);
				report_error(__FILE__, __func__, line);
				exit(RCEC_CORRUPT_MAPPING);
			}

			if (rc) { parts.at(1) = seq_revcom(parts.at(1)); }
			res[parts.at(1)] = parts.at(0);
		}
	}
	in.close();
	return res;
}

/* public member wrapper to do the job
 * call this from app
 * takes no arguments
 * */
void Read_counter::count(bool in_z) {
	// clear hashes just in case we are re-using the object
	counts_hash->clear();
	translated->clear();

	vector<umss> maps;
	for (size_t i = 0; i < read_maps.size(); ++i) {
		maps.push_back(load_mapping(read_maps.at(i), rcs.at(i)));
	}

	umss sample_hash = load_mapping(sample_map, idxrc);

	// input not zipped
	if (!in_z) {
		ifstream i1;
		if (infile) { attach_stream<ifstream>(infile, i1, std::ios_base::in); }
	
		// setup threads for collapsing the IDs
		boost::thread_group tgroup1;
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup1.create_thread(
					boost::bind(
						&Read_counter::collapse_reads<ifstream>,
						this,
						boost::ref(i1),
						boost::ref(sample_hash)
						)
					);
		}
		tgroup1.join_all();
		if (i1.is_open()) { i1.close(); }
	}

	// zipped input
	if (in_z) {
		igzstream z1;
		if (infile) { attach_stream<igzstream>(infile, z1, std::ios_base::in); }
	
		// setup threads for collapsing the IDs
		boost::thread_group tgroup1;
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup1.create_thread(
					boost::bind(
						&Read_counter::collapse_reads<igzstream>,
						this,
						boost::ref(z1),
						boost::ref(sample_hash)
						)
					);
		}
		tgroup1.join_all();
		if (z1.is_open()) { z1.close(); }
	}

	// setup threads for counting the reads
	boost::thread_group tgroup2;
	for (uint8_t i = 0; i < n_threads; ++i) {
		tgroup2.create_thread(
				boost::bind(
					&Read_counter::count_reads,
					this,
					boost::ref(maps)
					)
				);
	}
	tgroup2.join_all();
}

/* exports data into a user friendly tabular format
 * takes no arguments
 * */
void Read_counter::write_table() {
	uss* samples = new uss;
	uss* all_combos = new uss;
	multi_hash* ct = new multi_hash;
	multi_hash& ctr = *ct;

	vector<string> chunks;
	string record;

	// get a set of sample names and a set of all combos we see
	for (umsi_it it = translated->begin(); it != translated->end(); ++it) {
		record = it->first;
		chunks = split_string(record, HASH_SEP);
		samples->insert(chunks.at(0));
		all_combos->insert(chunks.at(1));
	}

	// initialize a hash of hasesh and populate with zeros for counts
	for (uss_it it1 = samples->begin(); it1 != samples->end(); ++it1) {
		ctr.insert(make_pair(*it1, umsi()));
		for (uss_it it2 = all_combos->begin(); it2 != all_combos->end(); ++it2) {
			ctr[*it1].insert(make_pair(*it2, 0));
		}
	}

	// fill in the hash of hashes with the real data
	for (umsi_it it = translated->begin(); it != translated->end(); ++it) {
		record = it->first;
		chunks = split_string(record, HASH_SEP);
		ctr[chunks.at(0)][chunks.at(1)] = it->second;
	}

	// open output file
	ofstream out;
	if (outfile) { attach_stream<ofstream>(outfile, out, std::ifstream::out); }

	// print the headeer row to output file and get sample names into a vector so we
	// keep a consistent order
	vector<string> ss;
	if (out.is_open()) { out << "combo"; }
	else { cout << "combo"; }
	for (uss_it it = samples->begin(); it != samples->end(); ++it) {
		if (out.is_open()) { out << OUTPUT_SEP << *it; }
		else { cout << OUTPUT_SEP << *it; }
		ss.push_back(*it);
	}
	
	if (out.is_open()) { out << "\n"; }
	else { cout << "\n"; }

	// iterate over the hash of hases and print the data to the output
	for (uss_it it = all_combos->begin(); it != all_combos->end(); ++it) {
		if (out.is_open()) { out << *it; }
		else { cout << *it; }

		for (size_t i = 0; i < ss.size(); ++i) {
			if (out.is_open()) { out << OUTPUT_SEP << ctr[ss[i]][*it]; }
			else { cout << OUTPUT_SEP << ctr[ss[i]][*it]; }
		}

		if (out.is_open()) { out << "\n"; }
		else { cout << "\n"; }
	}
	
	// close output
	if (out.is_open()) { out.close(); }

	// cleanup
	delete(all_combos);
	delete(samples);
	delete(ct);
}

/* writes the raw counts to a file
 * takes no arguments
 * */
void Read_counter::write_raw_counts() {
	ofstream out;
	if (outfile) { attach_stream<ofstream>(outfile, out, std::ifstream::out); }

	// print output
	for (umsi_it iti = translated->begin(); iti != translated->end(); ++iti) {
		string rec = iti->first;
		rec.replace(rec.find(HASH_SEP),1,OUTPUT_SEP);
		if (out.is_open()) { out << rec << OUTPUT_SEP << iti->second << endl; }
		else { cout << rec << OUTPUT_SEP << iti->second << endl; }
	}
	
	if (out.is_open()) { out.close(); }
}

void Read_counter::write_raw_stats() {
	ofstream out;
	if (stats_file) { attach_stream<ofstream>(stats_file, out, std::ifstream::out); }

	for (umsi_it it = stats_idx->begin(); it != stats_idx->end(); ++it) {
		if (out.is_open()) {
			out << "idx" << OUTPUT_SEP << it->first << OUTPUT_SEP << it->second << endl;
		} else {
			cout << "idx" << OUTPUT_SEP << it->first << OUTPUT_SEP << it->second << endl;
		}
	 }

	for (size_t i = 0; i < stats_r.size(); ++i) {
		for (umsi_it it = stats_r.at(i)->begin(); it != stats_r.at(i)->end(); ++it) {
			if (out.is_open()) {
				out << "r" << i+1 << OUTPUT_SEP << it->first << OUTPUT_SEP << it->second << endl;
			} else {
				cout << "r" << i+1 << OUTPUT_SEP << it->first << OUTPUT_SEP << it->second << endl;
			}
		}
	}

	if (out.is_open()) { out.close(); }
}
