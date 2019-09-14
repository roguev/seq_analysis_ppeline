#!/usr/bin/env python

import pandas as pd
import numpy as np	
import argparse
import os

def read_data(filename):
	'''reads the data file'''
	read_len = 0
	
	# initialize dictionaries first,
	# we will turn them into dataframes at the end
	# once they are populated
	NT = dict()
	QUAL = dict()
	LEN = dict()
    
	# open file and read line by line
	f=open(filename)
	for line in f.readlines():
		line = line.rstrip()
		tokens = line.split("\t")

		if tokens[0] == 'READ_LENGTH':
			read_len = int(tokens[1])

		if tokens[0] == 'NT_FREQ':
			NT[int(tokens[1])] = tokens[2:]

		if tokens[0] == 'QUAL':
			QUAL[int(tokens[1])] = tokens[2]

		if tokens[0] == 'LEN':
			LEN[int(tokens[1])] = tokens[2]
            
	f.close()
	
	# return data as dataframes, turn everything to numbers
	return(read_len,
		pd.DataFrame(data=NT.values(),
		index=NT.keys(),columns=list('ATGCN')).apply(pd.to_numeric),
		pd.DataFrame(data=QUAL.values(), index=QUAL.keys(),columns=['Q']).apply(pd.to_numeric),
		pd.DataFrame(data=LEN.values(), index=LEN.keys(),columns=['L']).apply(pd.to_numeric))

def plot_nt_freq(ax, df, N_thres=0.8,with_seq=False,show_thres=True):
	'''create a nucleotide frequency plot'''
	
	# sort dataframe
	df.sort_index(inplace=True)
	
	rl = df.shape[0]
	
	# try to guess the sequence
	if with_seq:
		# find max frequency for each position
		# this is a pd.Series with values of column names (nucleotides)
		# from the original df
		m_inds = df.idxmax(axis=1)
		
		# change the letter to an N if frequency lower than N_thres
		for pos in m_inds.index:
			if df.loc[pos, m_inds[pos]] < N_thres:
				m_inds[pos] = 'N'

		# labels become the values of the series
		label = m_inds.values
	else:
		label = None

	# build a stacked bar graph
	# plot As
	ax.bar(np.arange(1,rl+1),
		df['A'],
		align='center',
		color='green',
		alpha=.65)

	# plot Ts
	ax.bar(np.arange(1,rl+1),
		df['T'],
		bottom=df['A'],
		align='center',
		color='red',
		alpha=.65)
	
	# plot Gs
	ax.bar(np.arange(1,rl+1),
		df['G'],
		bottom=df['A']+df['T'],
		align='center',
		color='black',
		alpha=.65)
	
	# plot Cs
	ax.bar(np.arange(1,rl+1),
		df['C'],
		bottom=df['A']+df['T']+df['G'],
		align='center',
		color='blue',
		alpha=.65)
	
	# plot Ns
	ax.bar(np.arange(1,rl+1),
		df['N'],
		bottom=df['A']+df['T']+df['G']+df['C'],
		align='center',
		color='grey',
		alpha=.65,tick_label=label)

	# cosmetics
	ax.tick_params(labelsize=14)
	ax.set_xlim([0,rl+1])
	ax.set_ylim([0,1])
	ax.yaxis.grid(True, which='major')
	if show_thres:
		ax.axhline(N_thres, linestyle='--', color='black', linewidth=4)

	# set ylabel
	y_label = 'Fraction'
	ax.set_ylabel(y_label,fontsize=14)



def plot_qual(ax,df,Q_thres=30,show_thres=True):
	''' create a quality plot'''
	
	# sort dataframe
	df.sort_index(inplace=True)
	
	rl = df.shape[0]
	
	# create column for data below Q_thres
	df['below_T'] = np.zeros(rl)
	
	# move data below Q_thres to below_T column
	for pos in df.index:
		if df.loc[pos,'Q'] < Q_thres:
			df.loc[pos,'below_T'] =  df.loc[pos,'Q']
			df.loc[pos,'Q'] = 0

	# build a stacked bar graph
	# plot stuff above Q_thres in green
	ax.bar(np.arange(1,rl+1),
		df['Q'],
		align='center',
		color='green',
		alpha=.5)
	
	# plot stuff below_T in grey	
	ax.bar(np.arange(1,rl+1),
		df['below_T'],
		align='center',
		color='grey',
		bottom=df['Q'],
		alpha=.65)	
    
	# cosmetics
	ax.tick_params(labelsize=14)
	ax.set_xlim([0,rl+1])
	ax.set_ylim([20,45])
	ax.yaxis.grid(True, which='major')
	if show_thres:
		ax.axhline(Q_thres, linestyle='--', color='red', linewidth=4)

	# set ylabel
	y_label = 'Average quality'
	ax.set_ylabel(y_label,fontsize=14)



def main():	
	
	# parse command line
	parser = argparse.ArgumentParser()
	parser.add_argument("infile", help='input file')
	parser.add_argument("-q", "--q_thres", help='quality threshold (30)', default=35)
	parser.add_argument("-n", "--n_thres", help='frequency threshold for calling Ns (0.8)', default=0.8)
	parser.add_argument("-t","--hide_thres", help='hide (do not show) thresholds',action='store_true', default=False)
	parser.add_argument("-o", "--outfile", help='output file')
	args=parser.parse_args()
	
	N_thres = float(args.n_thres)
		
	Q_thres = float(args.q_thres)

	show_thres = not args.hide_thres
	
	# check if we can plot on screen if no outfile specified
	if "DISPLAY" not in os.environ:
		if args.outfile == None:
			print('No DISPLAY defined and no outfile. Exiting')
			quit()
		else:
			import matplotlib
			print('No DISPLAY defined. Setting backend to Agg')
			matplotlib.use('Agg')		
	
	# create initial dataframes to feed into functions afterwards
	r,n,q,l = read_data(args.infile)
	
	# setup graphics
	import matplotlib.pyplot as plt
	fig,ax = plt.subplots(3,1,figsize=(20,10))
	
	# plot nucleotide frequency
	plot_nt_freq(ax[0],n, N_thres=N_thres,show_thres=False)

	# prepare sata for the second plot
	m_inds = n.idxmax(axis=1)
	
	#dataframe with te same shape as n but filled with 0s
	n1 = pd.DataFrame(data=np.zeros(n.shape),index=n.index, columns=n.columns)
	
	# copy data below N_thres to the N column of n1 from n or to respective
	# column otherwise
	for pos in m_inds.index:
		if n.loc[pos,m_inds[pos]] < N_thres:
			n1.loc[pos,'N'] = n.loc[pos,m_inds[pos]]
		else:
			n1.loc[pos,m_inds[pos]] = n.loc[pos,m_inds[pos]]
    
    # plot second nucleotide frequency plot with the filtered data
	plot_nt_freq(ax[1],n1,N_thres=N_thres,with_seq=True,show_thres=show_thres)
	plot_qual(ax[2],q,Q_thres=Q_thres,show_thres=show_thres)
	
	# decide if we do screen or file
	if args.outfile == None:
		plt.show(fig)
	else:
		fig.savefig(args.outfile)
	
	# cleanup	
	plt.close(fig)



if __name__ == "__main__":
	main()
