#!/usr/bin/env python3

'''
parse all jsonl data in each 8 zones for mobile node 2 and get a csv file for knn training
'''

import json,csv
import sys

zones = {} # schema zone -> []

for z in range (1,9):
	zones[z] = []
	with open(f'z{z}.json') as f:
		for line in f:
			if line.startswith('{'):
				try:
					d = json.loads(line.rstrip())
					if d['mobile_id'] == 2:
						zones[z].append((d['b1'],d['b2'],d['b3'],d['b1r'],d['b2r'],d['b3r']))
				except Exception as e:
					# print('line:', line)
					# print(e)
					# exit(0)
					pass
				
outfile = 'data.csv'
# csv header
with open(outfile, 'w') as f:
	f.write('zone,b1,b2,b3,b1r,b2r,b3r\n')
	for z in zones:
		for d in zones[z]:
			f.write(','.join([str(z)]+[str(x) for x in d]) + '\n')
print('written to', outfile, file=sys.stderr)