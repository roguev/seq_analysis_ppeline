#!/usr/bin/env python

import pandas as pd
import numpy as np
import argparse
import os
from mpl_toolkits.axes_grid1 import make_axes_locatable

# heatmap colormaps
cmaps = {'SEQ-A':'hsv',
         'SEQ-T' : 'hsv',
         'SEQ-G' : 'hsv',
         'SEQ-C' : 'hsv',
         'SEQ-N' : 'hot',
         'CLUST' : 'jet',
         'QUAL' : 'hot',
         'LEN' : 'jet'}

# heatmap colormap limits
clims = {'SEQ-A':(0, 0.4),
         'SEQ-T' : (0, 0.4),
         'SEQ-G' : (0, 0.4),
         'SEQ-C' : (0, 0.4),
         'SEQ-N' : (0, 0.4),
         'CLUST' : (None, None),
         'QUAL' : [30,40],
         'LEN' : (70, 80)}



def read_data(filename):
	""" Reads raw data into a dataframe and takes care of rugged entries
	"""
	f=open(filename)
	max_n = 0
	for line in f.readlines():
		n_fields = len(line.split("\t"))
		if n_fields > max_n:
			max_n = n_fields
	f.close()
	df = pd.read_table(filename,header=None, index_col=0,names=range(max_n))
	return df



def get_max_tile_size(df):
	""" Determines the maximum tile size to be applied when there are missing tiles
	"""
	max_x, max_y = 0,0
	for tag in df.index.unique():
		y,x = df.loc[tag].shape
		if x > max_x:
			max_x = x
		if y > max_y:
			max_y = y

	return (max_x, max_y)



def sync_2d_arrays(arr1,arr2,axis=None):
	""" Synchronyzes array dimensions along the axis before merging
	"""
	s1 = arr1.shape
	s2 = arr2.shape

	# adjust the number of rows
	if axis==0:
		if s1[0] > s2[0]:
			arr2 = np.append(arr2,np.full((s1[0]-s2[0],s2[1]), np.nan), axis=0)
		elif s1[0] < s2[0]:
			arr1 = np.append(arr1,np.full((s2[0]-s1[0],s1[1]), np.nan), axis=0)

	# adjust the number of columns
	if axis==1:
		if s1[1] > s2[1]:
			arr2 = np.append(arr2,np.full((s2[0],s1[1]-s2[1]), np.nan), axis=1)
		elif s1[1] < s2[1]:
			arr1 = np.append(arr1,np.full((s1[0],s2[1]-s1[1]), np.nan), axis=1)

	return (arr1,arr2)



def tokenize_tags(df):
	""" Tokanizes the line tags
	"""
	x = [(i[0], i[1], i[2], i[3]) for i in df.index.unique().str.split('_')]
	f_set = set([a[0] for a in x])
	r_set = set([a[1] for a in x])
	l_set = set([a[2] for a in x])
	t_set = set([a[3] for a in x])

	return(f_set, r_set, l_set, t_set)



def merge_tiles(df, t_arr, tile_x, tile_y, f, r, l, t_set):
	""" Mergers tiles data horizontally
	"""

	# t_set is the set of all tiles
	for ti,t in enumerate(sorted(t_set)):
		#start from second tile
		if ti > 0:
	
			# generate the tag
			tag = '_'.join([str(f),str(r), str(l), str(t)])
			
			# check if data exists in dataset, if not generate a nan array
			# with the correct dimensions (tile_x, tile_y) for merging
			if tag in df.index:
				a_arr = df.loc[tag]
			else:
				a_arr = np.full((tile_x, tile_y), np.nan)

			# make the 2 arrays with consistent row dimensions and merge them
			t_arr, a_arr = sync_2d_arrays(t_arr, a_arr, axis=0)
			t_arr = np.append(t_arr, a_arr, axis=1)

	return(t_arr)



def plot_data(fig, df, mode, ls=None, fs=None, rs=None, cmaps=None, clims=None, mask_qual=False):
	""" main plotting function
	"""
	# axes stuff for text positionoing
	bottom, top = 0, 1

	# tokenize the line tags
	f_set, r_set, l_set, t_set = tokenize_tags(df)
	print("Data\nF = %s\nR = %s\nL = %s\nT = %i\n" %(' '.join(f_set), ' '.join(r_set), ' '.join(l_set), len(t_set)))

	if ls == None:
		lanes = l_set
	else:
		lanes = [l for l in ls if str(l) in l_set]

	if fs == None:
		features = f_set
	else:
		features = [f for f in fs if str(f) in f_set]
		
	if rs == None:
		reads = r_set
	else:
		reads = [r for r in rs if str(r) in r_set]

	print("Plot\nF = %s\nR = %s\nL = %s\nT = %i\n" %(' '.join(features), ' '.join(reads), ' '.join(lanes), len(t_set)))
	
	if len(features) == 0 or len(lanes) == 0 or len(reads) == 0:
		print('Nothing to plot. Exiting.')
		quit()

	# get maximum tile size 
	tile_x, tile_y = get_max_tile_size(df)
	
	# setup graphics
	yx_scale = float(tile_y)/float(tile_x)
	x_size = len(t_set)
	y_size = yx_scale*float(len(features))*float(len(lanes))*float(len(reads))
	axarr = fig.subplots(len(features)*len(lanes)*len(reads), 1)
	fig.set_size_inches(x_size, y_size)
	print("Image dimensions: %i x %i" %(100*x_size, 100*y_size))
	
	# decide which order we plot
	if mode == 'by_feature':
		var1 = features
		var2 = lanes
	elif mode == 'by_lane':
		var1 = lanes
		var2 = features
	else:
		# do nothing, just exit
		print('Invalid mode. Exiting')
		quit()

	# count the tracks we plot e.g. feature_read_lane combinations
	track = 0

	for v1 in var1:
		for r in sorted(reads):
			for v2 in var2:
				if mode == 'by_feature':
					f = v1
					l = v2
				else:
					f = v2
					l = v1
				
				# generate the tag to extract he first tile dataset
				tag = '_'.join([str(f), str(r), str(l), str(list(sorted(t_set))[0])])

				if tag not in df.index:
					# create a nan array
					t_arr = np.full((tile_x, tile_y), np.nan)
				else:
					# first tile as a seed for appending
					t_arr = df.loc[tag]

				# merge the tiles
				t_arr = merge_tiles(df, t_arr, tile_x, tile_y, f, r, l, t_set)
				
				# mask low quality regions if requested
				if mask_qual and f == 'QUAL':
					t_arr[np.isnan(t_arr)] = 0
					t_arr[t_arr < clims[f][0]] = np.nan
				
				# plot data
				h_map = axarr[track].imshow(t_arr,
							aspect='equal',
							cmap=cmaps[f],
							interpolation='hamming')

				# clip colorspace
				h_map.set_clim(clims[f][0], clims[f][1])

				# set track title
				title = str(f)+' R'+str(r)+' L'+str(l)
				print('Plotting '+ title)
				axarr[track].text(0,
						0.5,
						title,
						horizontalalignment='right',
						verticalalignment='center',
						rotation=90,
						transform=axarr[track].transAxes,
						fontsize=10)

				# turn axis text off
				axarr[track].set_xticks([])
				axarr[track].set_yticks([])

				# add colorbar
				divider = make_axes_locatable(axarr[track])
				cax = divider.append_axes("right", size=.2, pad=0.2)
				cb = fig.colorbar(h_map, cax=cax)
				cb.ax.tick_params(labelsize=8)

				track += 1
	
	# squish graph together as much as possible
	# adjust these parameters to change the way the figure looks 
	fig.subplots_adjust(hspace=.03,
			wspace=0,
			left=.01,
			right=.99,
			bottom=0.01,
			top=.99)
	
	return fig



def main():	

	# parse command line
	parser = argparse.ArgumentParser()
	parser.add_argument("mode", help='mode: by_lane or by_feature')
	parser.add_argument("infile", help='input file')
	parser.add_argument("-f", "--features", nargs='+', help='features to be plotted (all)')
	parser.add_argument("-l", "--lanes", nargs='+', help='features to be plotted (all)')
	parser.add_argument("-r", "--reads", nargs='+', help='reads to be plotted (all)')
	parser.add_argument("-q", "--q_lims", nargs='+', help='lower and upper bound of quality (30, 40)')
	parser.add_argument("-m","--mask_qual", help='masks low quality data as missing (False)',action="store_true",default=False)
	parser.add_argument("-o", "--outfile", help='output file')
	
	args=parser.parse_args()
	
	# set quality bounds
	if args.q_lims is not None:
		for qi, q_lim in enumerate(args.q_lims):
			clims['QUAL'][qi] = float(q_lim)
	
	# check if we can plot on screen if no outfile specified
	if "DISPLAY" not in os.environ:
		if args.outfile == None:
			print('No DISPLAY defined and no outfile. Exiting.')
			quit()
		else:
			import matplotlib
			print('No DISPLAY defined. Setting backend to Agg')
			matplotlib.use('Agg')		
	
	# setup graphics
	import matplotlib.pyplot as plt
	fig = plt.figure()
	
	# set nan to lightgrey for plotting
	for k,v in cmaps.iteritems():
		plt.cm.get_cmap(v).set_bad(color='lightgrey')

	# generate plot 
	fig = plot_data(fig,
			read_data(args.infile),
			args.mode,
			ls=args.lanes,
			fs=args.features,
			rs=args.reads,
			cmaps=cmaps,
			clims=clims,
			mask_qual=args.mask_qual)

	# show or save file depending on command line arguments
	if args.outfile == None:
		plt.show()
	else:
		fig.savefig(args.outfile)

	# free memory
	plt.close(fig)

if __name__ == "__main__":
	main()
