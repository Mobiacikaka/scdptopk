import pandas as pd

df1 = pd.read_csv(open('dataset_TSMC2014_NYC.txt'), sep='\t', header=None,encoding = "ISO-8859-1")
df2 = pd.read_csv(open('dataset_TSMC2014_TKY.txt'), sep='\t', header=None,encoding = "ISO-8859-1")

df1.columns = ['userid','venid','vencatid','venname','lat','long','tz','time']
df1 = df1.drop(['vencatid','venname','lat','long','tz','time'], axis=1)
df1['val'] = 1
df1.drop_duplicates(inplace=True)

df2.columns = ['userid','venid','vencatid','venname','lat','long','tz','time']
df2 = df2.drop(['vencatid','venname','lat','long','tz','time'], axis=1)
df2['val'] = 1
df2.drop_duplicates(inplace=True)

df1 = df1.groupby(['venid'], as_index=False)['val'].sum()
df2 = df2.groupby(['venid'], as_index=False)['val'].sum()

with open("../test/server/dataset.txt", 'w') as f:
	for i in range(int(df1.size/3)):
		f.write(str(df1['venid'][i])+"\t"+str(df1['val'][i])+"\n")

with open("../test/client/dataset.txt", 'w') as f:
	for i in range(int(df2.size/3)):
		f.write(str(df2['venid'][i])+"\t"+str(df2['val'][i])+"\n")
