from matplotlib import pyplot as plt
import numpy as np

kmax = 51

def read_result(folder_name):
	average_runtime_list = []
	for k in range(1, kmax):
		runtime_list = []
		for trial in range(0, 5):
			filename = folder_name + '/k=' + str(k) + '/' + str(trial) + ".cli.out"
			file = open(filename,'r')
			read_list = file.read().splitlines()
			runtime_list.append([float(i) for i in read_list])
		
		runtime_list = sum(map(np.array, runtime_list)) / 5
		average_runtime_list.append(runtime_list)

	return average_runtime_list

result1 = read_result('exp_1000')
result2 = read_result('exp_10000')
result3 = read_result('exp_100000')

def plot_result(result1, result2, result3, slot):
	slot1 = []
	slot2 = []
	slot3 = []
	for i in range(0, kmax-1):
		slot1.append(result1[i][slot])
		slot2.append(result2[i][slot])
		slot3.append(result3[i][slot])

	fig, ax = plt.subplots()
	k_list = range(1, kmax)
	ax.plot(k_list, slot1, label="10^3")
	ax.plot(k_list, slot2, label="10^4")
	ax.plot(k_list, slot3, label="10^5")
	ax.set(xlabel='k', ylabel='seconds', title='')
	ax.grid()
	leg = ax.legend()
	plt.show()

plot_result(result1, result2, result3, 4)