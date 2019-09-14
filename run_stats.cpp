#include "matrix.h"
#include "run_stats.h"
#include <string>
#include "fastq_seq.h"
#include "utils.h"
#include "gzstream.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;
using namespace utils;

/*----------------------------------------------------*/
// namespace run_stats
Read* run_stats::_add_read(run_t* r, uint8_t i) {
	if (r->find(i) == r->end()) {
		r->insert(std::pair<uint8_t, Read*>(i, new Read()));
	} // if
	return r->at(i);
}

// add Lane
Lane* run_stats::_add_lane(read_t* r, uint8_t i) {
	if (r->find(i) == r->end()) {
		r->insert(std::pair<uint8_t, Lane*> (i, new Lane()));
	} // if
	return r->at(i);
}

// add Tile
Tile* run_stats::_add_tile(lane_t* l, uint16_t i) {
	if (l->find(i) == l->end()) {
		l->insert(std::pair<uint16_t, Tile*> (i, new Tile()));
	} // if
	return l->at(i);
}

// add View
View* run_stats::_add_view(tile_t* t, STAT kind, char NT) {
	uint16_t k = static_cast<uint16_t>(kind) + static_cast<uint16_t>(NT);
	if (t->find(k) == t->end()) {
		t->insert(std::pair<uint16_t, View*> (k, new View(kind, NT)));
	} // if
	return t->at(k);
}

View* run_stats::_add_view(tile_t* t, STAT kind) {
	uint16_t k = static_cast<uint16_t>(kind);
	if (t->find(k) == t->end()) {
		t->insert(std::pair<uint16_t, View*> (k, new View(kind)));
	} // if
	return t->at(k);
}
/*----------------------------------------------------*/
// Read
// add Lane
Lane* Read::add_lane(uint8_t n) {
	return run_stats::_add_lane(data, n);
}

// get Lane
Lane* Read::get_lane(uint8_t n) const {
	return run_stats::get_value<uint8_t, Lane*>(*data, n);
}

vector<uint8_t> Read::lane_ids() const {
	return run_stats::get_keys<uint8_t, Lane*>(*data);
}

/*----------------------------------------------------*/
// Lane
//add tile
Tile* Lane::add_tile(uint16_t n) {
	return run_stats::_add_tile(data, n);
}

// get tile
Tile* Lane::get_tile(uint16_t n) const {
	return run_stats::get_value<uint16_t, Tile*>(*data, n);
}

vector<uint16_t> Lane::tile_ids() const {
	return run_stats::get_keys<uint16_t, Tile*>(*data);
}

/*----------------------------------------------------*/
// Tile
// add view
View* Tile::add_view(STAT kind, char NT) { 
	return run_stats::_add_view(data, kind, NT);
}

View* Tile::add_view(STAT kind) { 
	return run_stats::_add_view(data, kind);
}

// get view
View* Tile::get_view(uint16_t n) const {
	return run_stats::get_value<uint16_t, View*>(*data, n);
}

vector<uint16_t> Tile::view_ids() const {
	return run_stats::get_keys<uint16_t, View*>(*data);
}

/*----------------------------------------------------*/
// Run_stats
/* setters */
void Run_stats::set_xbin_size(uint16_t i) { xbin_size = i; }
void Run_stats::set_ybin_size(uint16_t i) { ybin_size = i; }
void Run_stats::set_output_sep(const string& sep) { OUTPUT_SEP = sep; }
void Run_stats::set_load_factor(uint32_t i) { load_factor = i; }
void Run_stats::set_n_threads(uint8_t i) { n_threads = i; }

void Run_stats::add_stat(STAT kind, char NT) {
	uint16_t k = static_cast<uint16_t>(kind);
	stats.push_back(std::pair<uint16_t, char>(k, NT));
}

void Run_stats::add_stat(STAT kind) {
	uint16_t k = static_cast<uint16_t>(kind);
	stats.push_back(std::pair<uint16_t, char>(k, 0));
}

vector<uint8_t> Run_stats::read_ids() const {
	return run_stats::get_keys<uint8_t, Read*>(*data);
}

const stat_t& Run_stats::_stats() const {
	return stats;
}

// add a read
Read* Run_stats::add_read(uint8_t r) {
	return run_stats::_add_read(data, r);
}

// get a read
Read* Run_stats::get_read(uint8_t r) const {
	return run_stats::get_value<uint8_t, Read*>(*data, r);
}

// add a lane
Lane* Run_stats::add_lane(uint8_t r, uint8_t l) {
	if (Read* R = add_read(r)) {
		return R->add_lane(l);
	}
	return NULL;
}

// get a lane
Lane* Run_stats::get_lane(uint8_t r, uint8_t l) const {
	if (Read* R = get_read(r)) { return R->get_lane(l); }
	return NULL;
}

// add a tile
Tile* Run_stats::add_tile(uint8_t r, uint8_t l, uint16_t t) {
	if (Read* R = add_read(r)) {
		if (Lane* L = R->add_lane(l)) {
			if (Tile* T = L->add_tile(t)) {
				return T;
			}
		}
	}
	return NULL;
}

//get a tile
Tile* Run_stats::get_tile(uint8_t r, uint8_t l, uint16_t t) const {
	if (Lane* L = get_lane(r,l)) { return L->get_tile(t); }
	return NULL;
}

// add a view
View* Run_stats::add_view(uint8_t r, uint8_t l, uint16_t t, STAT k, char NT) {
	if (Read* R = add_read(r)) {
		if (Lane* L = R->add_lane(l)) {
			if (Tile* T = L->add_tile(t)) {
				if (View* V = T->add_view(k, NT)) {
					return V;
				}
			}
		}
	}
	return NULL;
}

View* Run_stats::add_view(uint8_t r, uint8_t l, uint16_t t, STAT k) {
	if (Read* R = add_read(r)) {
		if (Lane* L = R->add_lane(l)) {
			if (Tile* T = L->add_tile(t)) {
				if (View* V = T->add_view(k)) {
					return V;
				}
			}
		}
	}
	return NULL;
}

// get a view
View* Run_stats::get_view(uint8_t r, uint8_t l, uint16_t t, uint16_t v) const {
	if (Tile* T = get_tile(r, l, t)) { return T->get_view(v); }
	return NULL;
}

/* print runtime parameters */
void Run_stats::print_params() const {
	cout << "Input:\t";
	if (infile) { cout << infile; } else { cout << "stdin"; } 
	cout << endl;

	cout << "Output:\t";
	if (outfile) { cout << outfile; } else { cout << "stdout"; }
	cout << endl;

	cout << endl;

	cout << "X bin:\t" << xbin_size << endl;
	cout << "Y bin:\t" << ybin_size << endl;
	cout << endl;

	cout << "Reads:\t";
	if (collect_reads.empty()) { cout << "all"; } 
	else {
		for (size_t l = 0; l < collect_reads.size(); ++l) { cout << +collect_reads.at(l) << " "; }
	}
	cout << endl;

	cout << "Lanes:\t";
	if (collect_lanes.empty()) { cout << "all"; } 
	else {
		for (size_t l = 0; l < collect_lanes.size(); ++l) { cout << +collect_lanes.at(l) << " "; }
	}
	cout << endl;

	cout << "Tiles:\t";
	if (collect_tiles.empty()) { cout << "all"; }
	else {
		for (size_t t = 0; t < collect_tiles.size(); ++t) { cout << collect_tiles.at(t) << " "; }
	}
	cout << endl;
	
	cout << "Stats:\t";
	for (size_t s = 0; s < stats.size(); ++s) {
		if (stats.at(s).first == static_cast<uint16_t>(STAT::CLUST)) { cout << "clust" << " "; }
		if (stats.at(s).first == static_cast<uint16_t>(STAT::QUAL)) { cout << "qual" << " "; }
		if (stats.at(s).first == static_cast<uint16_t>(STAT::LEN)) { cout << "len" << " "; }
		if (stats.at(s).first >= static_cast<uint16_t>(STAT::SEQ)) { cout << stats.at(s).second << " "; }
	}
	cout << endl;

	cout << "Output sep:\t\"" << OUTPUT_SEP << "\"" << endl;
	cout << "Threads:\t" << +n_threads << endl;
	cout << "Load factor:\t" << load_factor << endl;
}

/* print the tags of the collected data */
void Run_stats::print_collected() const {
	for (map<string, pair<double, double>>::const_iterator it = 
			collected.begin(); 
			it != collected.end(); 
			++it) {
		cout << "Data=" << it->first << "\t" << it->second.first << "\t" << it->second.second << endl;
	}
}

/* processed a sequence file
 * arguments
 * 	input stream
 * 	*/
template <class T1>
void Run_stats::process_seqs(T1& in) {
	vector<Fastq_seq>* seqs = new vector<Fastq_seq>;
	Fastq_seq fq;
	string id;
	string qual;
	string seq;

	double avg_qual;
	double avg_NTs;

	size_t xbin;
	size_t ybin;

	SEQ_HEADER sh;

	// temporary data
	run_t* t_Stats = new run_t;

	Read* t_Read = NULL;
	Lane* t_Lane = NULL;
	Tile* t_Tile = NULL;
	View* t_View = NULL;

	Read* nRead = NULL;
	Read* sRead = NULL;

	Lane* nLane = NULL;
	Lane* sLane = NULL;

	Tile* nTile = NULL;
	Tile* sTile = NULL;

	View* nView = NULL;
	View* sView = NULL;

	// holderes for lane, tile and view ids
	vector<uint16_t> tids;
	vector<uint16_t> vids;
	vector<uint8_t> lids;
	vector<uint8_t> rids;

	vector<uint8_t> c_reads = collect_reads;
	vector<uint8_t> c_lanes = collect_lanes;
	vector<uint16_t> c_tiles = collect_tiles;

	while (1) {
		seqs->clear();
		seqs->reserve(load_factor);

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

		for (size_t i = 0; i < seqs->size(); ++i) {
			rids.clear();
			lids.clear();
			tids.clear();
			vids.clear();

			t_Read = NULL;
			t_Lane = NULL;
			t_Tile = NULL;
			t_View = NULL;

			fq = seqs->at(i);
			id = fq.get_seq_id();
			seq = fq.get_seq();
			qual = fq.get_qual_str();

			sh = fq.parse_seq_id();

			// check if the sequence read is part of what we want e.g. read number, lane and tile
			// if these are not specified assumes we want it otherwise checks in the respective vectors 
			if (!c_reads.empty()) {
				if (std::find(c_reads.begin(), c_reads.end(), sh.read_number) == c_reads.end()) { continue; }
			}

			if (!c_lanes.empty()) {
				if (std::find(c_lanes.begin(), c_lanes.end(), sh.lane) == c_lanes.end()) { continue; }
			}

			if (!c_tiles.empty()) {
				if (std::find(c_tiles.begin(), c_tiles.end(), sh.tile) == c_tiles.end()) { continue; }
			}

			// determine the bin
			xbin = static_cast<size_t>(sh.x / xbin_size);
			ybin = static_cast<size_t>(sh.y / ybin_size);

			// increase by one so everything fits
			xbin += 1;
			ybin += 1;

			// add read if needed
			t_Read = run_stats::_add_read(t_Stats, sh.read_number);

			// add lane if needed
			t_Lane = t_Read->add_lane(sh.lane);
			
			// add tile if needed
			t_Tile = t_Lane->add_tile(sh.tile);
			
			// new empty tile, add views
			if (t_Tile->empty()) {
				for (size_t s = 0; s < stats.size(); ++s) {
					 t_View = t_Tile->add_view(
							 static_cast<STAT>(stats.at(s).first), 
							 stats.at(s).second);
				}
			}

			// expand views data if needed
			vids = t_Tile->view_ids();
			for (size_t v = 0; v < vids.size(); ++v) {
				if (t_View = t_Tile->get_view(vids.at(v)) ) {
					// rows
					if (t_View->_data()->r < ybin) {
						t_View->_data()->append_rows(ybin - t_View->_data()->r,0);
					} // if
					
					// columns
					if (t_View->_data()->c < xbin) {
						t_View->_data()->append_cols(xbin - t_View->_data()->c,0);
					} //if
				} // if
			} // for
		
			for (size_t v = 0; v < vids.size(); ++v) {
				if (t_View = t_Tile->get_view(vids.at(v)) ) {
		
					if (t_View->_kind() == STAT::CLUST) {
						t_View->_data()->at(ybin-1, xbin-1)++;
					} // if
	
					if (t_View->_kind() == STAT::LEN) {
						t_View->_data()->at(ybin-1, xbin-1) 
							+= static_cast<double>(seq.length());
					} // if
	
					if (t_View->_kind() == STAT::QUAL) {
						avg_qual = 0.0;
						for (size_t j = 0; j < qual.length(); ++j) { 
							avg_qual += static_cast<double>(qual.at(j)) - 33.0;
						}
						t_View->_data()->at(ybin-1, xbin-1) += 
							avg_qual/static_cast<double>(qual.length());
					} // if
		
					if (t_View->_kind() == STAT::SEQ) {
						avg_NTs = 0.0;
						for (size_t j = 0; j < seq.length(); ++j) {
							if (seq.at(j) == t_View->_NT()) { avg_NTs++; }
						} // for loop
						t_View->_data()->at(ybin-1, xbin-1) 
							+= avg_NTs/static_cast<double>(seq.length());
					} // if
				} // if
			} // for loop
		} // for loop
	} // while loop

	// critical
	// add data to main containers
	mtx.lock();

	// reset all the pointers
	nRead = NULL;
	sRead = NULL;
	
	nLane = NULL;
	sLane = NULL;

	nTile = NULL;
	sTile = NULL;

	nView = NULL;
	sView = NULL;

	// rset all the vectors
	rids.clear();
	lids.clear();
	tids.clear();
	vids.clear();

	//iterate over reads
	rids = run_stats::get_keys<uint8_t, Read*>(*t_Stats);
//	cout << "rids" << endl;
//	cout << "main sync loop" << endl;
	for (size_t r = 0; r < rids.size(); ++r) {
		sRead = add_read(rids.at(r));
		nRead = t_Stats->at(rids.at(r));

		// iterate over lanes
		lids = nRead->lane_ids();
//		cout << "lids" << endl;
		for (size_t l = 0; l < lids.size(); ++l) {
			// add lane to main structure if needed
			sLane = sRead->add_lane(lids.at(l));
			nLane = nRead->_data()->at(lids.at(l));

			// iterate over tiles in nLane
			tids = nLane->tile_ids();
//			cout << "tids" << endl;
			for (size_t t = 0; t < tids.size(); ++t) {
				// add tile if needed
				sTile = add_tile(rids.at(r), lids.at(l), tids.at(t));
	
				// populate tile if its newly added with views
				if (sTile->empty()) {	
					for (size_t s = 0; s < stats.size(); ++s) {	
						sView = add_view(
								rids.at(r),
								lids.at(l), 
								tids.at(t), 
								static_cast<STAT>(stats.at(s).first), 
								stats.at(s).second
								);	
					}// for
				} // if
			} // for
		
			// add new data to the main structure
			for (size_t t = 0; t < tids.size(); ++t) {
				sTile = get_tile(rids.at(r), lids.at(l), tids.at(t));
				nTile = nLane->_data()->at(tids.at(t));
			
				vids = nTile->view_ids();
				for (size_t v = 0; v < vids.size(); ++v) {
					sView = get_view(rids.at(r), lids.at(l), tids.at(t), vids.at(v));
					// cout << sView->_data()->r << " " << sView->_data()->c << endl;
					nView = nTile->_data()->at(vids.at(v));
	
					make_consistent(*(sView->_data()), *(nView->_data()),0.0);
					// cout << sView->_data()->r << " " << sView->_data()->c << endl;
					*(sView->_data()) += *(nView->_data());
				} // for loop views
			} // for loop tiles
		} // for loop lanes
	} // for loop reads
	
	mtx.unlock();
	// end critical
	// cleanup
	delete(seqs);
	delete(t_Stats);
}

/* the main function of the class. Call this from within your program
 * and before any of the stats print methods
 * arguments
 * 	bool showing if the onput is compressed or now
 * 	*/
void Run_stats::collect_stats(bool z_in) {
	boost::thread_group tgroup;
	
	ifstream i1;
	igzstream z1;
	
	// uncompressed input
	if (!z_in) {
		// attempt to open file
		if (infile) { attach_stream<ifstream>(infile, i1, std::ios_base::in); }
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Run_stats::process_seqs<ifstream>, 
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
		// ottempt to open file
		if (infile) { attach_stream<igzstream>(infile, z1, std::ios_base::in); }
		for (uint8_t i = 0; i < n_threads; ++i) {
			tgroup.create_thread(boost::bind(
						&Run_stats::process_seqs<igzstream>, 
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

/* print stats */ 
void Run_stats::output_stats() { 
	ofstream out;
	if (outfile) { attach_stream<ofstream>(outfile, out, std::ios_base::out); }

	Read* R = NULL;
	Lane* L = NULL;
	Tile* T = NULL;
	View* V = NULL;

	// used to store the cluster density view for normalization
	View* V_clust = NULL;

	vector<uint8_t> rids;
	vector<uint8_t> lids;
	vector<uint16_t> tids;
	vector<uint16_t> vids;
	uint16_t r_id = 0;
	uint16_t l_id = 0; 
	string R_ID;
	
//	cout << "output_stats" << endl;

	// get available read ids in a run
	rids = read_ids();
//	cout << "rids" << endl;
	for (size_t r = 0; r < rids.size(); ++r) {
		if (R = get_read(rids.at(r))) {
			r_id = static_cast<uint16_t>(rids.at(r));

			// get available lane ids in a read
			lids = R->lane_ids();
//			cout << "lids" << endl;
			for (size_t l = 0; l < lids.size(); ++l) {
				if (L = R->get_lane(lids.at(l))) {
					l_id = static_cast<uint16_t>(lids.at(l));

					// get available tile ids in a lane
					tids = L->tile_ids();
//					cout << "tids" << endl;
					for (size_t t = 0; t < tids.size(); ++t) {
						if (T = L->get_tile(tids.at(t))) {
					
							// get available view ids in a tile
							vids = T->view_ids();
//							cout << "vids" << endl;
							for (size_t v = 0; v < vids.size(); ++v) {
								R_ID.clear();
								if (V = T->get_view(vids.at(v))) {
							
									// depending o the kind ogf view output different tag
									switch(V->_kind()) {
										case STAT::CLUST : 
											R_ID += string("CLUST"); V_clust = V; break;
										case STAT::LEN : 
											R_ID += string("LEN"); break;
										case STAT::QUAL : 
											R_ID += string("QUAL"); break;
										case STAT::SEQ : 
											R_ID += string("SEQ-") + V->_NT(); break;
									} // switch

									// the row header
									R_ID += string("_") + 
										run_stats::to_string(r_id) +
										string("_") +
										run_stats::to_string(l_id) +
										string("_") +
										run_stats::to_string(tids.at(t));
									
									// get cluster density normalized figures
									if (V_clust && (V->_kind() != STAT::CLUST)) {
										*(V->_data()) /= *(V_clust->_data());
									}

									// add to collected data
									add_collected(R_ID, V);

									// add the row header to a string repesentation of each row and output
									// to out stream (output file)
									for (size_t r = 0; r < V->_data()->r; ++r) {
										if (outfile) {
											out << R_ID << OUTPUT_SEP << V->_data()->row_to_string(r, OUTPUT_SEP);
										} else {
											cout << R_ID << OUTPUT_SEP << V->_data()->row_to_string(r, OUTPUT_SEP);
										} // if
									} // for
								} // if V...
							} // for
						} // if T...
					} // for
				} // if L... 
			} // for
		} // if R..
	} // for
	// close out stream
	if (outfile) { out.close(); }
}
