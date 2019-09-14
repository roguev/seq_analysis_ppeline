#!/usr/bin/perl

# generates a gnuplot script for plotting sequence statistics using the output
# of the get_seq_stats module.
use strict;
use warnings;

# input parameters
my $in = shift;		#file
my $min_q = shift;	# min quality
my $max_q = shift;	# max quality

my $read_len;
my $mode = 0;

# settings tags
my $RL_tag = 'READ_LENGTH';
my $mode_tag = 'MODE';

# data tags
my $NT_tag = 'NT_FREQ';
my $len_tag = 'LEN';
my $qual_tag = 'QUAL';

# temporary data containers
my @line_arr;
my %nt_hash;
my @called_bases;

# parse the input
open(IN, $in);
while (my $line = <IN>) {
	chomp($line);
	
	# remporary containers	
	my @sorted_nt;
	my $nt;
	
	# search for tags
	if ($line =~ m/$RL_tag/ || $line =~ m/$mode_tag/ || $line =~ m/$NT_tag/) {
		@line_arr = split(/\t/,$line);
		
		# set read length
		if ($line_arr[0] eq $RL_tag) { $read_len = $line_arr[1]; }

		# set mode
		if ($line_arr[0] eq $mode_tag) { $mode = $line_arr[1]; }
	
		# do the base calling for the labels on top of the frequency panel
		if ($line_arr[0] eq $NT_tag) {
			$nt_hash{'A'} = ($line_arr[2] eq '-nan') ? 0 : $line_arr[2];
			$nt_hash{'T'} = ($line_arr[3] eq '-nan') ? 0 : $line_arr[3];
			$nt_hash{'G'} = ($line_arr[4] eq '-nan') ? 0 : $line_arr[4];
			$nt_hash{'C'} = ($line_arr[5] eq '-nan') ? 0 : $line_arr[5];
			$nt_hash{'N'} = ($line_arr[6] eq '-nan') ? 0 : $line_arr[6];
			
			# sort by value descending and pick the largest one
			@sorted_nt = sort { $nt_hash{$b} <=> $nt_hash{$a} } keys %nt_hash;
			$nt = $sorted_nt[0];
			
			# call the bases
			# cutoffs are arbitrary here
			if ($nt_hash{$nt} >= .85) { push(@called_bases, $nt); }
			
			if ($nt_hash{$nt} < .85 && $nt_hash{$nt} >= .7) { push(@called_bases, lc($nt)); }
			
			if ($nt_hash{$nt} < .7) { push(@called_bases, 'N'); }
		}
	}
}
close(IN);

# blurb
print "# Base calls: @called_bases\n";

# determine number of rows in the figure deciphering the mode
my $n_rows = 0;

# test for set bits
if ($mode >= 16) { $mode = 16; $n_rows = 2; }
if (($mode & 2) != 0) { $n_rows++; }
if (($mode & 4) != 0) { $n_rows++; }
if (($mode & 8) != 0) { $n_rows++; }
if ($n_rows > 2) { $n_rows = 2; }

# blurb
print "# Read len:\t$read_len\n";
print "# Rows:\t$n_rows\n";

# set dimensions
# 50 and 500 are arbitrary
my $xsize = $read_len*50;
my $ysize = $n_rows*500;
my $min_l = $read_len - 6;
my $max_l = $read_len + 2;

# set terminal etc
print 'set term png enhanced font "DejaVu Sans Mono," size '.$xsize.','.$ysize."\n";
print "set multiplot\n";

# nucleotide frequency plot with labels
if ( (($mode & 2) != 0) || (($mode & 16) != 0) ) {
	print "unset style\n";
	print "unset xrange\n";
	print "unset yrange\n";
	print "unset xlabel\n";
	print "unset ylabel\n";
	print "unset y2label\n";
	print "unset key\n";
	print "unset grid\n";

	
	print "set style data histogram\n";
	print "set style histogram rowstacked\n";
	print "set style fill transparent solid 0.7 border -1\n";
	print "set linetype 1 lc rgb 0x7CFC00\n";
	print "set linetype 2 lc rgb 0xF4425F\n";
	print "set linetype 3 lc rgb 0x3d3d3d\n";
	print "set linetype 4 lc rgb 0x4169E1\n";
	print "set linetype 5 lc rgb 'white'\n";
	print 'set xlabel "position" enhanced offset 0,.5'."\n";
	print 'set ylabel "fraction" enhanced offset 2,0'."\n";
	print "set key at screen .88,.9\n";
	
	my $label_counter = 0;
	my $lxo = 0;
	my $lyo = .915;
	for (my $i = 0; $i < scalar(@called_bases); $i++) {
		$label_counter++;
		# initial offset (.05) + chart pad(read_len + 1) * relative len of graph + offset for n-th label - centering offset
		$lxo = .05 + .75/($read_len+1) + ($label_counter-1)*.75/($read_len+1) - (.75/($read_len+1))/5; 
		print 'set label '.$label_counter.' "'.$called_bases[$i].'" at screen '.$lxo.', screen '.$lyo.' font ",14"'."\n";
	}
	
	print "set grid\n";
	print "set tmargin screen .9\n";
	print "set bmargin screen .52\n";
	print "set lmargin screen .05\n";
	print "set rmargin screen .80\n";
	
	print 'set title "Nucleotide composition" offset 0,.5'."\n";
	print "set yrange [0:1]\n";
	print 'set xtics font ",10"'."\n";
	print 'plot "<grep '.$NT_tag.' '.$in.' | cut -f2-" using 2:xtic(1) title '."'A'".", '' using 3 title 'T', '' using 4 title 'G', '' using 5 title 'C', '' using 6 title 'N'\n";
}

# quality plot
if ( (($mode & 4) != 0) || (($mode & 16) != 0) ) {
	print "unset grid\n";
	print "unset style\n";
	print "unset xrange\n";
	print "unset yrange\n";
	print "unset xlabel\n";
	print "unset ylabel\n";
	print "unset y2label\n";
	print "unset key\n";
	
	print "set grid\n";
	print "set linetype 1 lc rgb 0x90EE90\n";
	print "set style fill transparent solid 0.5 border -1\n";
	print 'set xlabel "position" enhanced offset 0,.3'."\n";
	print 'set ylabel "avg quality" enhanced offset 2,0'."\n";
	
	print "set tmargin screen .42\n";
	print "set bmargin screen .1\n";
	print "set lmargin screen .05\n";
	print "set rmargin screen .80\n";
	
	print 'set title "Quality per position" offset 0,-.8'."\n";
	print 'set xrange [-1:'.$read_len.']'."\n";
	print 'set yrange ['.$min_q.':'.$max_q.']'."\n";
	print 'set xtics font ",10"'."\n";
	print 'plot "<grep '.$qual_tag. ' '.$in.' | cut -f2-" using 2:xtic(1) with boxes'."\n";
}

# read length histogram
if ( (($mode & 8) != 0) || (($mode & 16) != 0) ) {
	print "unset grid\n";
	print "unset style\n";
	print "unset xrange\n";
	print "unset yrange\n";
	print "unset xlabel\n";
	print "unset ylabel\n";
	print "unset y2label\n";
	print "unset key\n";
	
	print "set grid\n";
	print "set linetype 1 lc rgb 0xBFBFBF\n";
	print "set style fill transparent solid 0.5 border -1\n";
	print 'set xlabel "length" enhanced offset 0,.5'."\n";
	print 'set y2label "fraction" enhanced'."\n";
	
	print "set tmargin screen .4\n";
	print "set bmargin screen .1\n";
	print "set lmargin screen .85\n";
	print "set rmargin screen .95\n";
	
	print 'set title "Length\ndistribution"'."\n";
	print 'set xrange ['.$min_l.':'.$max_l.']'."\n";
	print "set yrange [0:1]\n";
	print 'set xtics font ",6"'."\n";
	print 'plot "<grep '.$len_tag.' '.$in.' | cut -f2-" using 2:xtic(1) with boxes'."\n";
}
