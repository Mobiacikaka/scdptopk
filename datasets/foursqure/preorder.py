# %%
import pandas as pd
import numpy as np

df1 = pd.read_csv(open('dataset_TSMC2014_NYC.txt'), sep='\t', header=None,encoding = "ISO-8859-1")
df2 = pd.read_csv(open('dataset_TSMC2014_TKY.txt'), sep='\t', header=None,encoding = "ISO-8859-1")

df = pd.concat([df1, df2], ignore_index=True)

# %%
srv = open('server.txt', 'w')
cli = open('client.txt', 'w')

def write_element(file, element):
	i = 0
	for item in element:
		file.write(str(item))
		i = i + 1
		if i < 8:
			file.write('\t')
	file.write('\n')

i = 0
for item in df.iterrows():
	if np.random.randint(0, 10) % 2 == 0:
		write_element(srv, df.iloc[i])
	else:
		write_element(cli, df.iloc[i])
	i = i + 1
