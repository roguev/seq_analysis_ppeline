#!/usr/bin/perl
# generates a gnuplot batch script for plotting run statistics using the output
# of the get_run_stats module. xs and ys in the range of 200 to 500 work ok

use strict;
use warnings;

# input parameters
my $in = shift;		#file
my $min_q = shift;	# min quality
my $max_q = shift;	# max quality
my $min_l = shift;	# min length
my $max_l = shift;	# max length
my $xs = shift;		# tile x-size in pixels
my $ys = shift;		# tile y-size in pixels

# arrays etc to help process the data
my @clust_min, my @clust_max;

# tags used to pick relevant data
# should match the tags used in the datafile
my $tag = 'Data=';
my $clust_tag = 'CLUST_';
my $len_tag = 'LEN_';
my $qual_tag = 'QUAL_';
my $A_tag = 'SEQ-A_';
my $T_tag = 'SEQ-T_';
my $G_tag = 'SEQ-G_';
my $C_tag = 'SEQ-C_';
my $N_tag = 'SEQ-N_';
my $out_tag = 'Output:';

# temporary variables for parsing the input
my @line_arr;
my $data_file;

# used for storing the individual tiles labels
my %data_tree;
my %views;

sub update_hashes(\%\%$) {
	# passed by reference
	my $v = $_[0];
	my $dt = $_[1];
	my $k = $_[2];

	# reset captures
	"a" =~ /a/;
	
	# extract tokens
	$k =~ /([A-Za-z-]+_)(\d+)_(\d+)_(\d+)/;
	if ($1 ne '' && $2 ne '' && $3 ne '' && $4 ne '') {
		# update tables
		$v->{$1}++;
		$dt->{$2}->{$3}->{$4}++;
	}
}

# parse the input file
open(IN, $in) or die "Cannot open $in\n";
while (my $line = <IN>) {
	# look for the name of the data file
	if ($line =~ m/$out_tag/) {
		chomp($line);
		@line_arr = split("\t", $line);
		$data_file = $line_arr[1];
	}
	
	# look for data annotations
	if ($line !~ m/$tag/) { next; }
	
	# shave off 'Data=' tag and remove newline
	$line =~ s/$tag//;
	chomp($line);
	
	@line_arr = split("\t", $line);
	&update_hashes(\%views, \%data_tree, $line_arr[0]);
	
	# if its a cluster view collect min and max data as well
	if ($line_arr[0] =~ m/$clust_tag/) {
		push(@clust_min, $line_arr[1]);
		push(@clust_max, $line_arr[2]);
	}
}
close(IN);

# if there is a cluster view get find global minimum and maximum
if (exists $views{$clust_tag}) {
	@clust_min = sort {$a <=> $b} @clust_min;	# find minimum
	@clust_max = sort {$b <=> $a} @clust_max;	# find maximum
}

# get maximum number of tiles
my @n_tiles;
my $total_lanes = 0;
foreach my $r (sort {$a<=>$b} keys %data_tree) {
	foreach my $l (sort {$a<=>$b} keys %{$data_tree{$r}}) {
		# increment number of lanes
		$total_lanes++;
		
		# get tile number in the lane
		push(@n_tiles, scalar(keys $data_tree{$r}{$l}));
	}
}

# dertemine x-dimension
# get max number of tiles 
@n_tiles = sort {$b <=> $a} @n_tiles;
my $max_tiles = $n_tiles[0];

# total x-size
my $xsize = $xs*($max_tiles+2);		# + 2

# determine y-dimension
# individual lane size
my $lane_ysize = $ys*scalar(keys %views) + $ys;	# + 1

# total y-size
my $ysize = $lane_ysize*$total_lanes;

# write to script file initial canvas setup and multiplot
print "set term png size ".$xsize.",".$ysize."\n";
print "set multiplot\n";
print 'set datafile missing "-nan"'."\n";

# prepare the labels
my $label = '';		# empty string
my $lxo = 0;		# label x-origin
my $lyo = 0;		# label y-origin
my $label_c = 0;	# global label counter
my $lane_c = 0;		# counts lanes
my $label_lane = 0;	# label counter within each lane

foreach my $r (sort keys %data_tree) {
	foreach my $l (sort {$a <=> $b} keys %{$data_tree{$r}}) {
		$lane_c++;
		
		# set the y-origin
		$lyo = 1 - ($lane_c-1)*($lane_ysize)/$ysize - .8*$ys/$ysize;
		
		# reset counter
		$label_lane = 0;
		foreach my $t (sort {$a <=> $b} keys %{$data_tree{$r}{$l}}) {
			$label_c++;
			$label_lane++;
			
			# set x-origin
			$lxo = $xs/$xsize + ($label_lane-1)*$xs/$xsize + .1*$xs/$xsize;
			
			# form the label
			$label = $r.'.'.$l.'.'.$t;

			# write to script
			print "#\n# labels setup\n#\n";
			print "# $label\n";
			print 'set label '.$label_c.' "'.$label.'" at screen '.$lxo.', screen '.$lyo.' font ",24"'."\n";
		} # foreach my $t...
	} # foreach my $l...
} # forach my $r...

# main script writing function
sub draw_view($$$$$) {
	my $tag = $_[0];		# data tag
	my $min = $_[1];		# min for the colorbar
	my $max = $_[2];		# max for the colorbar
	my $palette = $_[3];	# palette
	my $lane_label = $_[4];	# row (lane) label
	my $row_c = $_[5];

	# set counters
	my $lane_c = 0;
	my $read_c = 0;
	
	# holds the grep search pattern beow
	my $search_pat = '';
	
	# used to hold tile id's
	my @tiles;
	
	# used a lot in calculations so only calculate once
	# $lane_ysize and $ysize are global vars
	my $ls_ys = $lane_ysize/$ysize;
	
	# %views is global
	if (exists $views{$tag}) {
		$row_c++;
		
		# %data_tree is global
		foreach my $r (sort {$a <=> $b} keys %data_tree) {
			foreach my $l (sort {$a<=> $b} keys %{$data_tree{$r}}) {
				$lane_c++;
				
				# set various initial coordinates for the plotting
				my $t_margin = 1 - $ls_ys*($lane_c-1) - $ls_ys*($row_c-1)/(scalar(keys %views) + 1) - $ys/$ysize;
				my $b_margin = $t_margin - $ys/$ysize;							
				my $l_margin = $xs/$xsize;										# padded on the left
				my $r_margin = $l_margin + $xs/$xsize;			
				
				# colorbar origins x and y
				my $cb_ox = 0;
				my $cb_oy = 0;
				
				# colorbar sizes x and y
				# $xs, $ys $xsize and $ysize are global
				my $cb_sx = $xs/$xsize; 
				my $cb_sy = $ys/$ysize;
				
				$cb_oy = $b_margin;			# stays the same
				print "unset colorbox\n";
				
				# do the plotting
				@tiles = (sort {$a <=> $b} keys %{$data_tree{$r}{$l}});
				for (my $i = 0; $i < scalar(@tiles); $i++) {
					$search_pat = $tag.$r.'_'.$l.'_'.$tiles[$i];	# form grep search pattern
		
					print "#\n# $search_pat\n#\n";				# blurb
					
					# initial setup
					print "unset key\n";
					print "unset title\n";
					print "unset xlabel\n";
					print "unset ylabel\n";
					
					# set margins for the heatmap
					print 'set tmargin at screen '.$t_margin."\n";
					print 'set bmargin at screen '.$b_margin."\n";
					print 'set lmargin at screen '.$l_margin."\n";
					print 'set rmargin at screen '.$r_margin."\n";
					$cb_ox = $r_margin;
					$l_margin = $r_margin;
					$r_margin = $l_margin + $xs/$xsize;
					print "unset xtics\n";
					print "unset ytics\n";
					
					# set the colorbar range and palette
					print "set cbrange [".$min.":".$max."]\n";
					print 'set palette model '.$palette."\n";
					
					# build the command using grep from the shell
					print 'cmd = ';
					print "'<grep ";
					print $search_pat;
					print " $data_file"."'.";
					print "' | cut -f2-'"."\n";		# cut the 1st column
					print 'print cmd'."\n";
					
					# if last tile then print together with colorbar
					if ($i == scalar(@tiles)- 1) {
						print "#\n";
						$cb_sx *= .4;
						$cb_sy *= .8;
						$cb_ox += .2*$xs/$xsize;
						$cb_oy += .1*$ys/$ysize;
						print 'set colorbox vertical user origin '.$cb_ox.','.$cb_oy.' size '.$cb_sx.','.$cb_sy."\n";
					}
					
					# if 1st tile then add the ylabel
					if ($i == 0) { print 'set ylabel "'.$lane_label.'" font ",30" offset -5,0'."\n"; }
					
					# main plotting command
					print 'plot cmd matrix with image'."\n";
					print "#\n";
				} # for...
			} # foreach my $l...
		} # foreach my $r...
	} # if
	
	# unset the tile labels, speeds up the script
	# $label_c is global 
	print "#\n# unsetting labels\n#\n";
	for (my $l = 1; $l <= $label_c; $l++) { print 'unset label '.$l."\n"; }
	return $row_c;
} # sub..

my $rowcounter = 0;	# keeps track of which row within the lane we are in

my $clust_palette = 'RGB rgbformulae 22,13,-31';
my $qual_palette = 'RGB rgbformulae 21,22,23';
my $N_palette = 'RGB rgbformulae 21,22,23';
#my $NT_palette = 'HSV rgbformulae 3,2,2';
my $NT_palette = 'RGB defined';
my $len_palette = 'RGB rgbformulae 22,13,-31';

# do the plotting for all 8 possible features. change the order to change the order in which they appear in the graph
$rowcounter = &draw_view($clust_tag,	$clust_min[0],	$clust_max[0],	$clust_palette, 'CLUSTER\nDENSTIRY', $rowcounter);
$rowcounter = &draw_view($qual_tag,		$min_q,			$max_q,			$qual_palette,	'QUALITY', $rowcounter);
$rowcounter = &draw_view($N_tag, 		0,				.2,				$N_palette,		'fraction\nNs', $rowcounter);
$rowcounter = &draw_view($A_tag,		.2,				.4,				$NT_palette,	'fraction\nAs', $rowcounter);
$rowcounter = &draw_view($T_tag,		.2,				.4,				$NT_palette,	'fraction\nTs', $rowcounter);
$rowcounter = &draw_view($G_tag,		.2,				.4,				$NT_palette,	'fraction\nGs', $rowcounter);
$rowcounter = &draw_view($C_tag,		.2,				.4,				$NT_palette,	'fraction\nCs', $rowcounter);
$rowcounter = &draw_view($len_tag,		$min_l,			$max_l,			$len_palette,	'LENGTH', $rowcounter);
