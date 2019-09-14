#!/bin/bash
echo Compiling: get_run_stats
echo g++ -O2 get_run_stats.cpp utils.cpp gzstream.cpp fastq_seq.cpp run_stats.cpp matrix.h gzboost.cpp -o get_run_stats -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x
g++ -O2 get_run_stats.cpp utils.cpp gzstream.cpp fastq_seq.cpp run_stats.cpp matrix.h gzboost.cpp -o get_run_stats -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x

echo Compiling: get_seq_stats
echo g++ -O2 get_seq_stats.cpp utils.cpp gzstream.cpp fastq_seq.cpp seq_stats.cpp gzboost.cpp -o get_seq_stats -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x
g++ -O2 get_seq_stats.cpp utils.cpp gzstream.cpp fastq_seq.cpp seq_stats.cpp gzboost.cpp matrix.h -o get_seq_stats -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x

echo Compiling: extract_reads
echo g++ -O2 extract_reads.cpp utils.cpp gzstream.cpp fastq_seq.cpp read_extractor.cpp gzboost.cpp -o extract_reads -lboost_thread -lboost_system -lpcrecpp -lz -lboost_iostreams -std=gnu++0x
g++ -O2 extract_reads.cpp utils.cpp gzstream.cpp fastq_seq.cpp read_extractor.cpp gzboost.cpp -o extract_reads -lboost_thread -lboost_system -lpcrecpp -lz -lboost_iostreams -std=gnu++0x

echo Compiling: combine_R1_R2
echo g++ -O2 combine_R1_R2.cpp utils.cpp fastq_seq.cpp map_merger.cpp gzstream.cpp gzboost.cpp -o combine_R1_R2 -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x
g++ -O2 combine_R1_R2.cpp utils.cpp fastq_seq.cpp map_merger.cpp gzstream.cpp gzboost.cpp -o combine_R1_R2 -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x

echo Compiling: get_unpaired
echo g++ -O2 get_unpaired.cpp utils.cpp fastq_seq.cpp map_merger.cpp gzstream.cpp gzboost.cpp -o get_unpaired -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x
g++ -O2 get_unpaired.cpp utils.cpp fastq_seq.cpp map_merger.cpp gzstream.cpp gzboost.cpp -o get_unpaired -lboost_thread -lboost_system -lz -lboost_iostreams -std=gnu++0x

echo Compiling: count_combos
echo g++ -O2 count_combos.cpp utils.cpp read_counter.cpp gzstream.cpp -o count_combos -lboost_thread -lboost_system -lz -std=gnu++0x
g++ -O2 count_combos.cpp utils.cpp read_counter.cpp gzstream.cpp -o count_combos -lboost_thread -lboost_system -lz -std=gnu++0x
