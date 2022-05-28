# %%
import numpy as np
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

df = pd.concat([df1,df2])
df.drop_duplicates(inplace=True)

df = df.groupby(['venid'], as_index=False)['val'].sum()
df = df.sort_values(by='val', ascending=False)
venids = df['venid'].values
values = df['val'].values

venids = venids.tolist()
values = values.tolist()

# %%

# P = []
# S = []
# kmin=1
# kmax=51
# for k in range(kmin, kmax):
# 	venid_topk = venids[:k]
# 	value_topk = values[:k]
# 	output = []
# 	c = 0
# 	s = 0
# 	for i in range(10):
# 		outfile = open("../exp2/"+str(k)+"/"+str(i)+".out")
# 		lines = outfile.readlines()
# 		for line in lines:
# 			line = line.strip("\n")
# 			if line in venid_topk:
# 				c = c + 1
# 				s = s + values[venid_topk.index(line)]
# 	c /= 10
# 	s /= 10
# 	P.append(c/k)
# 	S.append(s/sum(value_topk))

# print(P)
# print(S)

# # %%
# from matplotlib import pyplot as plt
# k_list = range(kmin, kmax)

# def plot_results(TS, ylab, eps, nr_trials, k_list):
#     """
#     Shows plot comparing results of TS and LD
#     """
#     SMALL_SIZE = 10
#     MEDIUM_SIZE = 12
#     BIGGER_SIZE = 14

#     plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
#     plt.rc('axes', titlesize=MEDIUM_SIZE)    # fontsize of the axes title
#     plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
#     plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
#     plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
#     plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize
#     plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title

#     fig, ax = plt.subplots()
#     ax.plot(k_list, TS, label="SC-TS")
#     ax.set(xlabel='k', ylabel=ylab, title=str(nr_trials) + ' trials, for epsilon of ' + str(eps))
#     ax.grid()
#     leg = ax.legend()
#     plt.show()

# plot_results(P, "P", 1.0, 10, k_list)
# plot_results(S, "S", 1.0, 10, k_list)


# %%

# %%

k = 20
venid_topk = venids[:k]
value_topk = values[:k]
c = 0
s = 0

for i in range(10):
    outfile = open("../exp/30/"+str(i)+".out")
    lines = outfile.readlines()
    for line in lines:
        line = line.strip("\n")
        if line in venid_topk:
            c += 1
            s += values[venid_topk.index(line)]
    
c /= 10 * k
s /= 10 * sum(value_topk)

print(c,s)

# %%
