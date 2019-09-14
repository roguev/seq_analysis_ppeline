Documentation for sequence analysis pipeline
============================================

The pipeline consists of 6 independent multithreaded modules:
(1)	get_seq_stats	-	extracts basic sequence stats like per-base
				nucleotide composition, sequence quality and
				read length distribution

(1.1)	ss_scripter.pl	-	a perl script for generating gnuplot batch
				scripts for plotting the output of the
				get_seq_stats module  				

(1.2)	plot_seq_stats.py - 	a python script for plotting the output of
				the get_seq_stats module

(2)	get_run_stats	-	extracts various run statistics such as
				cluster density and quality metrics on the
				flow cell surface by binning the data in a
				user specified way

(2.1)	rs_scripter*.pl -	2 perl scripts for generating gnuplot batch
				scripts for plotting the data using the output
				of the get_run_stats module

(2.2)	plot_run_stats.py - 	a python script forplotting the output of
				the get_run_stats module

(3)	extract_reads	-	extracts meaningful sequence from raw FASTQ 
				files

(4)	combine_R1_R2	-	matches paired-end reads

(5)	get_unpaired	-	finds unpaired reads

(6)	count_combos	-	maps sequences to human readable IDs and counts 
				them

All six modules are able to process input in either raw text of compressed
.gz format. All modules output in raw text format and modules 3,4 and 5 also 
output in compressed .gz format.

Modules (1), (2), (3) and (6) are able to process input from STDIN and all
modules can output to STDOUT. This allows for dasy-chaining different modules.

Performance scales linearly with the number of running threads when the input
is in flat text format. Outputting in .gz format does not affect performance
in a very significant way. All modules output a table of run parameters to
stdout which can be re-directed to a file for logging purposes.

================================================================================

get_seq_stats
============
This module extracts useful statistics from raw FASTQ files. These include
per-base nucleotide composition, sequence quality score as well as read length
distribution. The module can be very useful when dealing with lower quality data
and can be instrumental when picking anchor and spacer sequences used in the
extract_reads module.
The module outputs delimited tabular data to a user-specified output file.

Command line arguments
......................

Mandatory
---------

--r_len, -l
	read length. This is usually specified during the sequencing run. This
	parameter is important as it affects the binning in the read length
	distribution analysis as well as deciding which reads are to be
	considered - reads longer than the specified read length will be
	ignored.


Optional
--------

--in, -i
	input FATSQ file (raw text or .gz). If ommited input will be taken
	from STDIN. Only uncomressed input can be fed in thru STDIN so when
	utilizing this method one has to pipe thru zcat.

--raw, -r
	a flag for setting the input data format to raw (the output of
	the read_extrator module). Set to false by default

--out, -o
	output file. If ommited output is sent to STDOUT.

--stats, -s
	decide which stats to collect
		count	- read count
		freq	- nucleotide frequency at each position
		qual	- average per nucleotide quality (QC score)
		length	- read length distribution
		all	- all of the above
	
	Different stats can be combined by specifying the --stats, -s
	parameter several times like so:
		-scount -squal -sfreq

--in_sep, -I
	input file separator (tab by default)
	
--out_sep, -O
	output file delimiter (tab by default)

--threads, -t
	number of runing threads. Larger number improves performance. Default 
	is set to 4

--load, -f
	this parameter controls how many sequences are processed at once by 
	each thread and can improve performance. Defaults to 25,000.

--quiet, -q
	suppress parameters output. Useful when output is directed to
	STDOUT.

.............
Example usage
.............

get_seq_stats -iR1.gz -oR1_stats.txt -l 75 -sfreq -squal
	
	Extract nucleotide frequency and quality stats from R1.gz using
	read length of 75 nucleotides. Write output to R1_stats.txt and log
	file to logs/R1_stats.log 

zcat R1.gz | get_seq_stats -l 75 -sfreq -squal -q > R1_stats.txt
	Same as above but utilizing the STDIN and STDOUT (with the --quiet,-q
	options suppressing parameters output).


--------------------------------------------------------------------------------
ss_scripter.pl
--------------------------------------------------------------------------------	
	This is a perl script for generating gnuplot compatible batch scripts for
	plotting the output of this module. It produces a plot of the data
	obtained. Takes 3 arguments:
		- an input file - the ouptut of get_seq_stats
		- 2 numbers denoting minimum and maximum quality for the
		  quality histigram
	
	The output of the script can be fed directly to gnuplot and will
	produce a PNG file (note: redirect the gnuplot output to a file)

Example usage:
	ss_scripter.pl seq_stats_R1.txt 20 40 | gnuplot > R1_stats.png

	will generate a script using data from seq_stats_R1.txt piping the
	output directly to gnuplot which will save the result in a file
	R1_stats.png


--------------------------------------------------------------------------------
plot_seq_stats.py
--------------------------------------------------------------------------------
	This is a python script for plotting the output of the get_seq_stats
	module. It requires panda, numpy and matplotlib to run. The output is
	either shown on the screen or savet to a file depending on input
	command line arguments. The script takes 4 arguments:
		- input file (mandatory) - this is the output of the
		  get_seq_stats module
		- -q, --q_thres - the quality threshold used when plotting.
		  Default is 30.
		- -n, --n_thres - the nucleotide frequency threshold for
		  calling N-bases. Default is 0.8
		- -o, --outfile - the output file. If non is pecified the
		  result is shown on the screen when possible.
		- -t, --hide_thres - supresses the plotting of the
		  threshold line on the graphs  

Example usage:
	plot_seq_stats.py seq_stats.txt -n 0.9 -q 35 -o seq_stats.png
		
		will generate a file seq_stats.png from seq_stats.txt using
		nucleotide frequency threshold of 0.9 and quality threshold of
		35.
	 
	 
================================================================================

get_run_stats
=============
This module extracts useful statistics from raw FASTQ files. The data is
binned in a user specified bins across the surface of the flow cell and then
stats like cluster density, per bin sequence quality and fraction of different
nucleoides is computed and output in a delimited tabular format. Performance scales
inverse proportionally with the bin size. Bin sizes between 250 and 1000 usually work
reasonably well. The defaults for bin sizes are both set to 500. A specific
lane and tile can be given as optional arguments.
Stats are output per lane and tile so the output files can be pretty bulky.

Command line arguments
......................

Mandatory
---------

	None.

Optional
--------

--in, -i
	input FATSQ file (raw text or .gz). If ommited input will be taken
	from STDIN. Only uncomressed input can be fed in thru STDIN so when
	utilizing this method one has to pipe thru zcat.

--out, -o
	output file. If ommited output is sent to STDOUT.

--xbin, -x
	size of the bin in the horizontal direction. Defaults to 500

--ybin, -y
	size of the bin in the vertical direction. Defaults to 500
	
	Bins in the 250 - 1000 range offer a good balance between performance
	and detail.

--stat, -s
	decide which stats to collect
		clust			- cluster density
		qual			- per bin sequence quality
		len			- per bin total sequence length
		A,T,G,C and N		- per bin fraction of A,T,G,C or N
	
	Different stats can be combined by specifying the --stats, -s
	parameter several times like so:
		-sclust -squal -sA -slen

--read, -R
	collect data for a specified read. If unused all reads will be
	processed

--lane, -L
	collect data for a specified lane. If unused all lanes will be
	processed

--tile, -T
	collect data for a specified tile. If unused all tiles will be
	processed

	For the --tile, -T and the -lane, -L parameters multiple values can be
	specified by applying the parameter several times like so:
		-L1 -L2 -T1101 -T1108

--out_sep, -O
	outpt file delimiter (tab by default)

--threads, -t
	number of runing threads. Larger number improves performance. Default 
	is set to 4

--load, -f
	this parameter controls how many sequences are processed at once by 
	each thread and can improve performance. Defaults to 25,000.

--quiet, -q
	suppress parameters output. Useful when output is directed to
	STDOUT.


.............
Example usage
.............

get_run_stats -i R1.gz -o R1_stats.txt -x500 -y500 -sclust -sN | tee logs/R1_stats.log
	
	Extract cluster density and Ns frequency fraction from R1.gz using xbin and ybin
	sizes of 500. Write output to R1_stats.txt and log file to logs/R1_stats.log

zcat R1.gz | get_run_stats -x500 -y500 -sclust -sN -q > R1_stats.txt
	Same as above but utilizing STDIN and STDOUT (and suppressing
	parameters output)
	

--------------------------------------------------------------------------------
rs_scripter*.pl
--------------------------------------------------------------------------------	
	These are a perl scripts for generating gnuplot compatible scripts for
	plotting the output of this module. They produce an annotated set of
	heatmaps representing the data collected. The scripts  accepts 7 parameters:
		- a file with the output of the get_run_stats module. Mote:
		  when used with the --quiet,-q parameter get_run_stats
		  produces no log information so in order to use the script
		  this option ought to be ommited.
		
		- 4 colorbar setting parameters
			- min and max quality
			- min and max length
		
		- 2 parameters setting the x and y size of the individual
		  heatmap
	The output of the script can be fed directly to gnuplot and will
	produce a PNG file (note: redirect the gnuplot output to a file)
	
	rs_scripter_img.pl produces batch scripts for heatmaps without
	interpolation. These are about twcice as fast to execute as the output
	from the rs-scripter_pm3d.pl which produces scripts for heatmaps with
	automatic interpolation,

Example usage:
(1)	gp_scripter_img.pl run_stats_log_R1.txt 20 40 70 80 250 250 | gnuplot >
	R1_stats.png

(2)	gp_scripter_pm3d.pl run_stats_log_R1.txt 20 40 70 80 250 250 | gnuplot >
	R1_stats.png

	will both generate batch scripts using data from run_stats_log_R1.txt piping
	the output directly to gnuplot which will save the result in a file
	R1_stats.png. (2) will take approximately 2x the time that (1) takes
	and will generate substantially larger output PNG file.


--------------------------------------------------------------------------------
plot_run_stats.py
--------------------------------------------------------------------------------
	This is a python script for plotting the output of the get_run_stats
	module. It requires panda, numpy and matplotlib to run. The output is
	either shown on the screen or savet to a file depending on input
	command line arguments. The script takes 8 arguments:
		- input file (mandatory) - this is the output of the
		  get_run_stats module
		- mode (mandatory) - affects the order in which the data is
		  being plotted. Possible vallues are 'by_lane' and
		  'by_feature'
		- -f, --features - features to plot. If ommited all features
		  are plotted
		- -l, --lanes - lanes to plot. If ommited all lanes are
		  plotted
		- -r, --reads - reads to be plotted.If ommited all reads are
		  plotted
		- -q, --q_lims - the lower and upper bound of the quality (for
		  quality plots). Affects the colormap bounds for the heatmap.
		  Defaults to (30, 40)
		- m, --mask_quall - cause the script to mask data with quality
		  lower than the lower bound as missing data. Default is
		  False.
		- -o, --outfile - the output file. If non is pecified the
		  result is shown on the screen when possible.

Example usage:
	plot_run_stats.py by_lane run_stats.txt -f CLUST QUAL -l 1 2 -r 2 -m
	-q 35 40 -o run_stats.png
		
		will generate a file run_stats.png from run_stats.txt plotting
		CLUST and QUAL for lanes 1 and 2 read 2 by lane using quality
		bounds of 35 and 40 and masking data with quality less than 35
	 
	 
================================================================================

extract_reads
=============
This moduke extracts meaningful sequence from FASTQ files. Supports input and 
outputin raw text or compressed .gz format. In its simplest form it it will
extract sequences looking like this:

	(left_anchor)-(region of interest (ROI) )-(right anchor)

It relies heavily on pattern matching and allows mismatches in the flanking 
anchor sequences. This module is very memory efficient.

Sequences are specified using A, T, C and G (in capital). 'x' is used to
signify a low quality position (usually represented by N) in the raw sequence
file. Internally it gets replaced by [ATCGN] pattern and allows for analysis
of sequencing runs where the read quality is not very high especially towards
the end of the sequence.

Command line arguments
......................

Mandatory
---------

--left, -l
	left anchor sequence as a string. Use "" when no left anchor is
	present.

--right, -r
	right anchor sequence as a string. Use "" when no right anchor is
	present.

--roi_min, -m
	minimum length of the region of interest between the left and right 
	anchor
		
--roi_max, -M
	maximium length of the region of interest between the anchors


Optional
--------

--in, -i
	input FATSQ file (raw text or .gz). If ommited input will be taken
	from STDIN. Only uncomressed input can be fed in thru STDIN so when
	utilizing this method one has to pipe thru zcat.

--out, -o
	output file. If ommited output is sent to STDOUT.

--spacer, -s
	spacer sequence as a string. When a spacer is specified it is implicit 
	that the number of ROIs =  number of spacers + 1. That is if only one
	spacer is  specified the extrated sequences look like this:
	
		(left_anchor)-(ROI_1)-(spacer)-(ROI_2)-(right_anchor)

--valid, -v
	output file containing the extracted valid sequences (e.g. sequences 
	that match the desired architecture). The format of the output file is a
	delimited table containing the following fields:

		UNIQUE_READ_ID, INDEX_SEQUENCE, MATCHED_ROI, ROI_QUALITY_STRING.....

	If more than one ROI is present then a pair of MATCHED_ROI, ROI_QUALITY_STRING
	for each of them will be written in the output.

--fastq_out, -F
	output valid reads in FASTQ format. All the ROIs will be concatenated
	into a single sequence

--rejected, -x
	output file containing the rejected reads. The format is a delimited 
	table containing the following fields:
		
		UNIQUE_READ_ID, INDEX_SEQUENCE, FASTQ_SEQUENCE, QUALITY STRING

	You must specify at least one of -v or -x (or both). Only one of the
	--valid,-v or --rejected,-x can be redirected to STDOUT.

--out_sep, -O
	outpt file delimiter (tab by default)

--threads, -t
	number of runing threads. Larger number improves performance. Default 
	is set to 15

--load, -f
	this parameter controls how many sequences are processed at once by 
	each thread and can improve performance

--no_mml, -L
	disallow mismatches in the left anchor, otherwise one mismatch is 
	allowed by default

--no_mmr, -R
	disallow mismatches in the right anchor, otherwise one mismatch is 
	allowed by default

--no_mms, -S
	disallow mismatches in the spacer, otherwise one mismatch is allowed by 
	default

--quiet, -q
	suppress parameters output. Useful when output is directed to
	STDOUT.

.............
Example usage
.............

extract_reads -iR1_L1.gz -vR1_L1_18_24.gz -l CACCTTGTTG -r GTTTAAGAGC -m 18 -M 
24 | tee logs/R1_L1_18_24.log &

	Take an input file R1_L1.gz and looks for sequences matching the 
	following pattern:

		CACCTTGTTG-([ATGCN]{18,24})-GTTTAAGAGC

	Write the output in R1_L1_18_24.gz. Also create a log file with the run 
	parameters.

zcat R1_L1.gz | extract_reads -v -xR1_reject.gz -l CACCTTGTTG -r GTTTAAGAGC -m 18 -M 
24 -q > R1_valid.gz
	
	Same as the first example but get input from STDIN, write rejected reads to 
	R1_reject.gz and valid reads to R1_valid.gz thru STDOUT suppressing parameters 
	output (-q option).
================================================================================

combine_R1_R2
=============
This module matches paired-end reads using the output of the extract_reads 
module (in either flat text or compressed .gz format). It does so by matching the
unique IDs of  individual reads and concatenating the result into a single output
(in either flat text or compressed .gz format). This module is a real memory hog as
it internally uses a hash to match read IDs.

Command line arguments
......................

Mandatory
---------

--in1, -1
	file containing the read1 sequences

--in2, -2
	file containing the read2 sequences

Optional
--------

--out, -o
	output file. The output is a delimited table (flat or .gz format) with 
	the following fields:

	UNIQUE_ID,IDX_READ1,SEQ_1_READ1,QUAL_1_READ1..SEQ_N_READ2,QUAL_N_READ2

	If there is more than one ROI per read these are directly concatenated 
	to produce the output

--in_sep, -I
	input file delimiter (tab by default). Should match the one used as 
	output delimiiter in the extract_reads module.

--out_sep, -O
	ouptut file delimiter (tab by default)

--lines, -l
	this determines the number of lines processed in a single cycle of the 
	program (default is 100M on a 64GB RAM machine). The program consumes
	about 3.4GB of RAM per 10M lines so this is an important consideration.
	The larger this parameter is the faster the program runs. Internally,
	in each cycle, the specified number of lines is loaded from the read1
	input and compared to the entire read2 input. The results are written
	to the output and the memory is released for use in the next cycle.

--threads, -t
	 number of runing threads. Larger number improves performance. Default 
	is set to 15

--load, -f
	this parameter controls how many lines are processed at once by each
	thread and can improve performance> Default is set to 10,000.

--quiet, -q
	suppress parameters output. Useful when output is directed to
	STDOUT.

.............
Example usage
.............

combine_R1_R2 -1 R1_L1_18_24.gz -2 R2_L1_18_24.gz -o R1R2_L1.gz | tee 
logs/R1R2_L1.log
	
	Take R1_L1_18_24.gz and R2_L1_18_24.gz and match paired reads. Write 
	output to R1R2_L1.gz. Also write a log file with the run parameters.


combine_R1_R2 -1 R1_L1_18_24.gz -2 R2_L1_18_24.gz -q > R1R2_L1.gz

	Same as the first example except output is redirected to a file
	R1R2_L1.gz thru STDOUT (parameters outut is suppressed with tre -q
	option).
================================================================================

get_unpaired
============
This module uses the same API as the combine_R1_R2 module and extracts unpaired 
reads from the output
of the extract_reads module.

Command line arguments
......................

Mandatory
---------

--in1, -1
	file containing the read1 sequences

--in2, -2
	file containing the read2 sequences

Optional
--------

--out1, -3
	output file containing the read1 sequences that cannot be matched to 
	read2 sequences. If ommited output is sent to STDOUT.

--out2, -4	
	output file containing the read2 sequences that cannot be matched to 
	read1 sequences. If ommited output is sent to STDOUT.

--in_sep, -I
	input file delimiter (tab by default). Should match the one used as 
	output delimiiter in the extract_reads module.

--out_sep, -O
	ouptut file delimiter (tab by default)

--threads, -t
	 number of runing threads. Larger number improves performance. Default 
	is set to 15

--load, -f
	this parameter controls how many lines are processed at once by each 
	thread and can improve performance. Default is set to 10,000.

--quiet, -q
	suppress parameters output. Useful when output is directed to
	STDOUT.
================================================================================

count_combos
============
This module takes the output from either extract_reads or combine_R1_R2 and 
generates a report of raw counts for each sequence defined in a separate sequence
mapping file. The report is also broken by sample specified in a separate sample
mapping file (more on this below).

Command line arguments
......................

Mandatory
---------

--smap, -s
	sample mapping file. This is a tab-delimited file in the following 
	format and with no header row:

		sample_ID	index_sequence

	For example:
		sample_1	ATGCTG
		sample_2	GTCGAT
	
	Only one sample mapping file can be specified per run.

--map, -m
	sequence mapping file. This is a delimited file with the following 
	format and with no header row:

		sequece_ID	sequence

	For example:
		ARID2_1	GGAACTGCCGCAGCTCGTCC
		ARID2_2	GAACCGGGGGGGCAGCGCCG
		ARID2_3	GGGGTCCCGGCTGACAAGTG
		ASH2L_1	GGAGCGGTCGCAAATGCAAC
		ASH2L_2	GCAGCCGCTCCTCCTGGAGA
		ASH2L_3	GTGGCCGTGATGGCGGCGGC

	More than one mapping file can be specified when more than one read/ROI 
	need to be mapped. In this case ROIs and  sequence mappings are matched
	sequentially e.g. the first ROI is analysd using the first mapping file,
	the second ROI - the second mapping file etc.

Optional
--------

--in, -i
	input file containing sequence data. This can be the output from either 
	extract_reads or combine_R1_R2 modules

--out, -o
	output file. This is a delimited text file in either flat text or 
	tabular format containing human readable sequence IDs (from a sequence
	mapping file), sample IDs (from a sample mapping file) and counts

--global, -g
	used to specify a global sequence mapping file. It takes an integer 
	specifying the number of ROIs/reads that will be processed. This is
	necassary as the program does not know up-front anything about the
	content of the input.

--rcr, -r
	reverse complement read. Takes an integer argument specifying which 
	read/ROI to reverse complement. Read/ROI counting starts from 1 e.g.
	-r2 means reverse complement read/ROI #2

--rci, -x
	a flag telling the porogram to reverse complement the index sequence. 
	If not specified the index sequence is left as is (default).

--rmm, -a
	number of allowed mismatches for reads/ROIs. These are specified for 
	each individual read sequentially e.g:
	
		...-a1 -a5 -a2...
	
	is interpreted as 1 mismatch for ROI_1, 5 mismatches for ROI_2 and 2 
	mismatches for ROI_3. The default is 1 for all reads/ROIs

--imm, -b
	number of allowed mismatces for the index sequence. It has to do with 
	mapping data to different samples. Default is 1.

--min_q, -q
	this parameter specifies the threshold per-base quality and in 
	combination with the --lq_base,-l parameter determines if a particular
	sequence is rejected due to poor quality. Default is 20.

--lq_base, -l
	maximim allowed number of positions with quality lower than the 
	threshold specified by the --min_q,-q parameter that will cause a sequence
	to be rejected due to bad quality. Default is 5.

--n_thr, -t
	number of running threads. Default is 15.

--col_bs, -u
	number of records the collapser sub-module will try to process at once 
	in each thread. Default is 250,000.

--cnt_bs, -w
	number of records the counter sub-module will try to process at once in 
	each thread. Default is 1000.

--no_undef, -Y
	a flag instructing the program to not output undefined (e.g. 
	unspecified sample) reads

--no_fail, -F
	a flag instructing the program to not output sequences rejected due to 
	poor quality

-no_c_fail, -C
	a flag telling the program to not collapse the quality failed reads 
	when outputing them. May be useful for tracking down sequencing problems

--no_unk, -U
	do not output unknown sequences e.g. sequences that do not match 
	anything in the supplied sequence mapping file

--undef_t, -Z
	tag used to denote undefined (e.g unknown index) sequences. Set to 
	'undef' by default.

--fail_t, -Q
	tag used to denote sequences rejected due to poor quality. Set to 
	'Q_FAIL' by default.

--unk_t, -X
	tag used to denote unknown sequences. Set to 'unknown' by default

--p_sep, -P
	delimiter used in the sequence ID in the output. Set to ':' by default.

--map_sep, -M
	delimiter used in the sequence mapping file. Can only be specified once 
	and applies for all mapping files. Set to 'tab' by default.

--in_sep, -I
	input file delimiter. If specified should match the output delimiter 
	for extract_reads/combine_R1_R2 modukes. Set to 'tab' by default.

--out_sep, -O
	output delimiter. Set to 'tab' by default.

--stats, -c
	also output raw stats e.g. raw counts for each sequence in the 
	analysis. Takes a filename argument. Can be useful when troubleshoouting
	a sequencing run. Outputs a table with the following columns:

		seq_type	sample_id	sequence	count

	seq_type can be idx (for index) or roi_id (r1 would be roi1). 
	sample_id, sequence and count should be self-explanatory.

--raw, -R
	use 'raw' output format. This is the default and it produces an output 
	in the following delimited format:
		
		sample_id	sequence_id	count

--table, -T
	use a table format for the output. The resulting table has the 
	following columns and a header row.

		sequence_id	count_in_sample_1......count_in_sample_n

--quiet, -v
	suppress parameters output. Useful when output is directed to
	STDOUT.
.............
Example usage
.............
count_combos -iR1R2_L2.gz -s smap -g2 -r2 -m sg_hs -oL2_counts_table.txt -U 
-F -Y -T | tee logs/L2_counts_table.log
	
		Take sequences in R1R2_L2.gz and count using a sample map 
		'smap' and a global mapping file 'sg_hs' applying for both
		ROIs (-g2 parameter). Reverse-complement the ROI_2 sequence(-r2).
		Output results as a table (-T) and do not output data for the
		unknown(-U), failed(-F) and undefined(-Y) sequences. Also write a
		log file with the run parameters.

zcat R1R2_L2.gz | ount_combos -s smap -g2 -r2 -m sg_hs -U -F -Y -T -q > L2_counts_table.txt

		Same as the first example but utilizing STDIN and STDOUT (and
		suppressing the parameters output)
