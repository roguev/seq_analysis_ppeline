#include <string>
#include <iostream>
#include <fstream>
#include "gzstream.h"
#include "gzboost.h"
#include "map_merger.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "utils.h"
using namespace std;
using namespace utils;

//Map_merger::Map_merger(char* i1, char* i2, int m_lines, int rthr, int wthr) {
/* constuctor
 * takes the input filenames for the 2 id mapping files and a
 * number of threads */
Map_merger::Map_merger(char* i1, char* i2, char* out) {
	R1fn = i1;
	R2fn = i2;
	outfile = out;
	unpR1 = NULL;
	unpR2 = NULL;

	lines_read = 0;

	//defaults
	max_lines = 75000000;
	n_threads = 10;
	load_factor = 10000;
	INPUT_SEP = "\t";
	OUTPUT_SEP = "\t";

	init_hashes();
	R1_hash->reserve(max_lines);
}

Map_merger::Map_merger(char* i1, char* i2, char* out, uint32_t mlines) {
	R1fn = i1;
	R2fn = i2;
	outfile = out;
	unpR1 = NULL;
	unpR2 = NULL;

	lines_read = 0;

	max_lines = mlines;
	n_threads = 10;
	load_factor = 10000;
	INPUT_SEP = "\t";
	OUTPUT_SEP = "\t";

	init_hashes();
	R1_hash->reserve(max_lines);
}

Map_merger::Map_merger(char* i1, char* i2, char* o1, char* o2) {   
	R1fn = i1;
	R2fn = i2;
	unpR1 = o1;
	unpR2 = o2;
	outfile = NULL;

	lines_read = 0;

	//defaults
	max_lines = 75000000;
	n_threads = 10;
	load_factor = 10000;
	INPUT_SEP = "\t";
	OUTPUT_SEP = "\t";

	init_hashes();
	R1_hash->reserve(max_lines);
}

void Map_merger::init_hashes() {
	/* initialize internal hashes on the heap */
	R1_hash = new umsvs;

	unpaired_R1 = new uss;
	unpaired_R2 = new uss;

	R1_ids = new uss;
	R2_ids = new uss;
}

/* destructor
 * cleans all the stuff we initialized on the heap in the cosntructor */
Map_merger::~Map_merger() {
	delete(R1_hash);
	delete(unpaired_R1);
	delete(unpaired_R2);
	delete(R1_ids);
	delete(R2_ids);
}

/* print parameters */
void Map_merger::print_params() {
	cout << "Inputs" << endl;
	cout << "R1:\t";
	if (R1fn) { cout << R1fn; } else { cout << "stdin"; }
	cout << endl;

	cout << "R2:\t";
	if (R2fn) { cout << R2fn; } else { cout << "stdin"; }
	cout << endl;
	cout << endl;
	
	cout << "Outputs:" << endl;
	cout << "unpaired R1:\t";
	if (unpR1) { cout << unpR1; } else { cout << "stdout"; }
	cout << endl;
	
	cout << "unpaired R2:\t";
	if (unpR2) { cout << unpR1; } else { cout << "stdout"; }
	cout << endl;
	
	cout << "merged maps:\t";
	if (outfile) { cout<< outfile; } else { cout << "stdout"; }
	cout << endl;
	cout << endl;

	cout << "input sep:\t\"" << INPUT_SEP << "\"" << endl;
	cout << "output sep:\t\"" << OUTPUT_SEP << "\"" << endl;
	cout << "max lines:\t" << max_lines << endl;
	cout << "load factor:\t" << load_factor << endl;
	cout << "N threads:\t" << +n_threads << endl;
}

/* setter for the maximum number of lines per iteration
 * higher numbers lead to more memory usage
 * default is 10,000,000 (see constructor */
void Map_merger::set_max_lines(uint32_t mlines) { max_lines = mlines; }

/* setter for the number of running threads */
void Map_merger::set_n_threads(uint8_t nthr) { n_threads = nthr; }

/* setter for the load factor */
void Map_merger::set_load_factor(uint32_t i) { load_factor = i; }

void Map_merger::set_input_sep(const string& sep) { INPUT_SEP = sep; }

void Map_merger::set_output_sep(const string& sep) { OUTPUT_SEP = sep; }
/* reads an id map
 * takes no arguments */
template<class T>void Map_merger::read_id_map(T& inR1) {
	string line;
	vector<string>* buffer = new vector<string>;

	// temporary hash
	umsvs* temp_map = new umsvs;

	vector<string> chunks;

	// check if stream is good and we have not reached the maximum number of lines to rad
	while (inR1.good() && (lines_read < max_lines)) {
		buffer->clear();
		buffer->reserve(load_factor);
		temp_map->clear();

		// critical
		// lock the mutex to read from the R1 stream
		// and fill in the buffer for processing
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if (lines_read < max_lines) {
				if (getline(inR1, line)) {
					buffer->push_back(line);
					lines_read++;
					} else { break; }
			}
		}
		mtx.unlock();
		// end critical

		// populate the temporary hash
		for (size_t j = 0; j < buffer->size(); ++j) {
			chunks = split_string(buffer->at(j), INPUT_SEP);
			// iterate over the chunks vector and add to the hash
			for (size_t k = 1; k < chunks.size(); ++k) {
				(*temp_map)[chunks.at(0)].push_back(chunks.at(k));
			}
		}

		// critical
		// lock the mutex to add to the main id hash
		mtx.lock();
		R1_hash->insert(temp_map->begin(), temp_map->end());;
		mtx.unlock();
		// end critical
		}
	// cleanup
	delete(temp_map);
	delete(buffer);
}

/* merges the id maps
 * takes no argumenst */
template<class T1> void Map_merger::match_reads(T1& inR2, ofstream& outf, bool z_out) {
	string line;
	vector<string>* in_buffer = new vector<string>;
	string* out_buffer = new string;

	vector<string> chunks;
	string key;

	// read from the R2 stream
	while (inR2.good()) {
		in_buffer->clear();
		in_buffer->reserve(load_factor);

		out_buffer->clear();
		out_buffer->reserve(500*load_factor);
	
		// critical
		// lock the mutex and fill in the in_buffer
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if ( getline(inR2, line ))  { in_buffer->push_back(line); } else { break; }
		}
		mtx.unlock();
		// end critical
		
		// iterate over the in_buffer to find matching reads in the main R1 hash
		for (size_t j = 0; j < in_buffer->size(); ++j) {
			chunks = split_string(in_buffer->at(j), INPUT_SEP);
			key = chunks.at(0);

			// check if we have the id already and if so add a new record to the
			// out buffer
			if (R1_hash->find(key) != R1_hash->end()) {
				// start with the key
				*out_buffer += key;
				
				// iterate ove the R1_hash
				for (size_t k = 0; k < (*R1_hash)[key].size(); ++k) {
					*out_buffer +=  OUTPUT_SEP + (*R1_hash)[key].at(k);
				}

				// iterate over the chunks vector
				for (size_t l = 1; l < chunks.size(); ++l) {
					*out_buffer += OUTPUT_SEP + chunks.at(l);
				}

				// add the newline at the end
				*out_buffer += "\n";
			}
		}

		if (z_out) { *out_buffer = compress_string(*out_buffer); }

		// critical
		// write the out buffer to cout
		mtx.lock();

		// to file
		if (outf.is_open()) { outf << *out_buffer; } 
		
		// to stdout
		else { cout << *out_buffer; }

		mtx.unlock();
		// end critical
	}
	// cleanup
	delete(in_buffer);
	delete(out_buffer);
}

/* a public member threaded wrapper function to match
 * R1 and R2 Ids. Call this from app */
void Map_merger::merge_id_maps(bool z_in, bool z_out) {
	// open files
	ifstream i1;
	ifstream i2;

	igzstream z1;
	igzstream z2;

	ofstream o;

	// open file if one is given
	if (outfile) { attach_stream<ofstream>(outfile, o, ios_base::out | ios_base::binary); }

	// input and output unzipped
	if (!z_in) {
		attach_stream<ifstream>(R1fn, i1, ios_base::in);
		attach_stream<ifstream>(R2fn, i2, ios_base::in);

		// read from R1
		while (i1.good()) {
			// reset line counter
			lines_read = 0;
		
			// setup threads
			boost::thread_group tgroup1;
			for (uint8_t i = 0; i < n_threads; ++i) {
				tgroup1.create_thread(boost::bind(
							&Map_merger::read_id_map<ifstream>, 
							this,
							boost::ref(i1)
							)
						);
			}
			tgroup1.join_all();
		
			// reset the R2 stream - for each iteration of the loop we iterate over the entire
			// R2 stream to try to find matches
			i2.clear();
			i2.seekg(0, ios::beg);
	
			// setup threads
			boost::thread_group tgroup2;
			for (uint8_t i = 0; i < n_threads; ++i) {
				tgroup2.create_thread(boost::bind(
							&Map_merger::match_reads<ifstream>, 
							this,
							boost::ref(i2),
							boost::ref(o),
							z_out
							)
						);
			}
			tgroup2.join_all();

			// empty the main R1 hash to be rady for the next iteration
			R1_hash->clear();
			}
		i1.close();
		i2.close();
	}

	// input zipped
	if (z_in) {
		attach_stream<igzstream>(R1fn, z1, ios_base::in);
	
		// read from R1
		while (z1.good()) {
			// reset line counter
			lines_read = 0;
		
			// setup threads
			boost::thread_group tgroup1;
			for (uint8_t i = 0; i < n_threads; ++i) {
				tgroup1.create_thread(boost::bind(
							&Map_merger::read_id_map<igzstream>, 
							this,
							boost::ref(z1)
							)
						);
			}
			tgroup1.join_all();
		
			// reopen the R2 stream - for each iteration of the loop we iterate over the entire
			// R2 stream to try to find matches
			attach_stream<igzstream>(R2fn, z2, ios_base::in);
			
			// setup threads
			boost::thread_group tgroup2;
			for (uint8_t i = 0; i < n_threads; ++i) {
				tgroup2.create_thread(boost::bind(
							&Map_merger::match_reads<igzstream>, 
							this,
							boost::ref(z2),
							boost::ref(o),
							z_out
							)
						);
			}
			tgroup2.join_all();
			z2.close();
			z2.clear();

			// empty the main R1 hash to be rady for the next iteration
			R1_hash->clear();
			}
		z1.close();
		z2.close();
	}

	// close files
	if (o.is_open()) { o.close(); }
}

/* extracts the IDs form a mapping file
 * arguments:
 * 	input filestream
 * 	set to store the results
 * 	*/
template<class T> void Map_merger::get_ids(T& in, uss& s) {
	string line;
	vector<string>* buffer = new vector<string>;

	// temporary set
	uss* temp_set = new uss;

	vector<string> chunks;
	
	// check if stream is ok
	while (in.good()){
		buffer->clear();
		buffer->reserve(load_factor);

		temp_set->clear();

		// critical
		// lock the mutex and read from the stream to populate the buffer
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if (getline(in, line)) {
				buffer->push_back(line);
			} else { break; }
		}
		mtx.unlock();
		// end critical

		// iterate over the buffer and extract the IDs and populate the temporary set
		for (size_t j = 0; j < buffer->size(); ++j) {
			chunks = split_string(buffer->at(j), INPUT_SEP);
			temp_set->insert(chunks.at(0));
		}

		// critical
		// insert temporary set into main set
		mtx.lock();
		s.insert(temp_set->begin(), temp_set->end());
		mtx.unlock();
		// end critical
		}
	// cleanup
	delete(temp_set);
	delete(buffer);
}

/* iterates over a file and extracts reads matching IDs
 * in a given set
 * arguments:
 * 	input stream
 * 	a set containing the IDs of interest
 * 	output stream
 * 	boolen zipped output
 * 	*/
template<class T1> void Map_merger::extract_reads(T1& in, uss& s, ofstream& out, bool z_out) {
	string line;
	vector<string> chunks;
	vector<string>* in_buffer = new vector<string>;
	string* out_buffer = new string;


	// check if stream is good
	while (in.good()) {
		in_buffer->clear();
		in_buffer->reserve(load_factor);

		out_buffer->clear();
		out_buffer->reserve(500*load_factor);

		// critical
		// read from input and fill buffer
		mtx.lock();
		for (uint32_t i = 0; i < load_factor; ++i) {
			if (getline(in, line)) {
				in_buffer->push_back(line);
			} else { break; }
		}
		mtx.unlock();
		// end critical
		
		// extract ID and check if it is in the set
		// if yes write the record to the output
		for (size_t j = 0; j < in_buffer->size(); ++j) {
			chunks = split_string(in_buffer->at(j), INPUT_SEP);
			
			if (s.find(chunks.at(0)) != s.end()) {
				*out_buffer += in_buffer->at(j) + "\n";
			}
		}

		if (z_out) { *out_buffer = compress_string(*out_buffer); }

		// critical
		// write output
		mtx.lock();
		
		// to file
		if (out.is_open()) { out << *out_buffer; }
		
		// to stdout
		else { cout << *out_buffer; }

		mtx.unlock();
		// end critical
	}
	delete(in_buffer);
	delete(out_buffer);
}

/* finds the difference between two sets
 * arguments:
 * 	s1 and s2 are sets to be diff'd
 * 	r is the resulting set
 * 	*/
void Map_merger::set_diff(uss& s1, uss& s2, uss& r) {
//	for (boost::unordered_set<string>::const_iterator it = s1.begin(); it != s1.end(); ++it) {
	for (uss_it it = s1.begin(); it != s1.end(); ++it) {
		if (s2.find(*it) == s1.end()) {
			r.insert(*it);
		}
	}
}

/* extracts unpaired reads from an experiment
 * arguments:
 * 	output filenames
 * 	*/
void Map_merger::get_unpaired_reads(bool z_in) {
	ifstream i1;
	ifstream i2;
	igzstream z1;
	igzstream z2;

	boost::thread_group tgroup1;
	boost::thread_group tgroup2;

	if (!z_in) {
		// open the R1 mapping
		attach_stream<ifstream>(R1fn, i1, ios_base::in);
	
		// setup threads
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup1.create_thread(boost::bind(
						&Map_merger::get_ids<ifstream>, 
						this, 
						boost::ref(i1), 
						boost::ref(*R1_ids)
						)
					);
		}
		tgroup1.join_all();

		// reset the R1 stream
		i1.close();
		i1.clear();

		// open R2 mapping
		attach_stream<ifstream>(R2fn, i2, ios_base::in);
		
		// setup threads
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup2.create_thread(boost::bind(
						&Map_merger::get_ids<ifstream>,
						this,
						boost::ref(i2),
						boost::ref(*R2_ids)
						)
					);
		}
		tgroup2.join_all();

		// reset R2 stream
		i2.close();
		i2.clear();
	}

	
	if (z_in) {
		// open the R1 mapping
		attach_stream<igzstream>(R1fn, z1, ios_base::in);
	
		// setup threads
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup1.create_thread(boost::bind(
						&Map_merger::get_ids<igzstream>, 
						this, 
						boost::ref(z1), 
						boost::ref(*R1_ids)
						)
					);
		}
		tgroup1.join_all();
		// close R1 stream. REMEMBER to reopen
		z1.close();
		z1.clear();

		// open R2 mapping
		attach_stream<igzstream>(R2fn, z2, ios_base::in);
		
		// extract all IDs from R2
		// setup threads
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup2.create_thread(boost::bind(
						&Map_merger::get_ids<igzstream>,
						this,
						boost::ref(z2),
						boost::ref(*R2_ids)
						)
					);
		}
		tgroup2.join_all();
		
		// close R2 stream. REMEMBER to reopen
		z2.close();
		z2.clear();
	}

	// run the set_diff on separate threds simultaneously to improve performance
	// find the difference between R1 and R2 IDs set
	boost::thread_group tgroup3;
	tgroup3.create_thread(boost::bind(
				&Map_merger::set_diff,
				this,
				boost::ref(*R1_ids), 
				boost::ref(*R2_ids),
				boost::ref(*unpaired_R1)
				)
			);

	// find the difference between R2 and R1 IDs
	tgroup3.create_thread(boost::bind(
				&Map_merger::set_diff,
				this,
				boost::ref(*R2_ids), 
				boost::ref(*R1_ids),
				boost::ref(*unpaired_R2)
				)
			);
	tgroup3.join_all();

	// extrack unpaired IDs
	boost::thread_group tgroup4;
	boost::thread_group tgroup5;
	
	ofstream o1;
	ofstream o2;

	// open output if files are given
	if (unpR1) { attach_stream<ofstream>(unpR1, o1, ios_base::out | ios_base::binary); }
	if (unpR2) { attach_stream<ofstream>(unpR2, o2, ios_base::out | ios_base::binary); } 
	
	if (!z_in) {
		// setup threads
		attach_stream<ifstream>(R1fn, i1, ios_base::in);
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup4.create_thread(boost::bind(
						&Map_merger::extract_reads<ifstream>,
						this,
						boost::ref(i1),
						boost::ref(*unpaired_R1),
						boost::ref(o1),
						true
						)
					);
		}
		tgroup4.join_all();
		i1.close();
		i1.clear();
		
		// setup threads
		attach_stream<ifstream>(R2fn, i2, ios_base::in);
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup5.create_thread(boost::bind(
						&Map_merger::extract_reads<ifstream>,
						this,
						boost::ref(i2),
						boost::ref(*unpaired_R2),
						boost::ref(o2),
						true
						)
					);
		}
		tgroup5.join_all();
		i2.close();
		i2.clear();
	}
	
	if (z_in) {
		attach_stream<igzstream>(R1fn, z1, ios_base::in);
		for (uint8_t i = 0; i < n_threads; ++i) {
			// setup threads
			tgroup4.create_thread(boost::bind(
						&Map_merger::extract_reads<igzstream>,
						this,
						boost::ref(z1),
						boost::ref(*unpaired_R1),
						boost::ref(o1),
						true
						)
					);
		}
		tgroup4.join_all();
		z1.close();
		z1.clear();
	
		attach_stream<igzstream>(R2fn, z2, ios_base::in);
		for (uint8_t i = 0; i < n_threads; ++i) {
			// setup a thread
			tgroup5.create_thread(boost::bind(
						&Map_merger::extract_reads<igzstream>,
						this,
						boost::ref(z2),
						boost::ref(*unpaired_R2),
						boost::ref(o2),
						true
						)
					);
		}
		tgroup5.join_all();
		z2.close();
		z2.clear();
	}

	// close files
	if (o1.is_open()) { o1.close(); }
	if (o2.is_open()) { o2.close(); }
}
