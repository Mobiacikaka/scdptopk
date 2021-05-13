# %%
import pandas as pd
import numpy as np

# %%
def random_output(filename, mode):
	df = pd.read_csv(open(filename), sep='\t', header=None,encoding = "ISO-8859-1")
	df.columns = ['userid','venid','vencatid','venname','lat','long','tz','time']
	df.drop_duplicates(inplace=True)
	srv = open("server.txt", mode)
	cli = open("client.txt", mode)
	for i in df.iterrows():
		if np.random.randint(0, 10) % 2 == 0:
			srv.write(str(i[1]['userid']) + "\t" + i[1]['venid'] + "\n")
		else:
			cli.write(str(i[1]['userid']) + "\t" + i[1]['venid'] + "\n")
	srv.close()
	cli.close()

random_output('dataset_TSMC2014_NYC.txt', 'w')
random_output('dataset_TSMC2014_TKY.txt', 'a')

# %%
def combine(filename):
	df = pd.read_csv(open(filename), sep="\t", header=None, encoding="ISO-8859-1")
	df.columns = ['userid','venid']
	df['val'] = 1
	df.drop_duplicates(inplace=True)
	df = df.groupby(['venid'], as_index=False)['val'].sum()
	with open(filename, "w") as f:
		for i in df.iterrows():
			f.write(i[1]['venid'] + "\t" + str(i[1]['val']) + "\n")

combine('server.txt')
combine('client.txt')

# %%
