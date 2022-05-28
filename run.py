#!/bin/python3
import os
from src.gm import generate_key

def build_scdptopk():
	os.system('mkdir -p build')
	os.system('cd build && cmake .. && make -j$(nproc)')

def copy_file(folder_name):
	os.system(f'mkdir -p {folder_name}')
	os.system(f'mkdir -p {folder_name}/server')
	os.system(f'mkdir -p {folder_name}/client')
	os.system(f'cp ./build/scdptopk {folder_name}/server')
	os.system(f'cp ./build/scdptopk {folder_name}/client')
	os.system(f'cp ./datasets/server.txt {folder_name}/server/dataset.txt')
	os.system(f'cp ./datasets/client.txt {folder_name}/client/dataset.txt')

def copy_key(folder_name):
	def print_key():
		f = open('key.txt', 'w')
		key_A = generate_key()
		n, y = key_A['pub']
		p, q = key_A['priv']
		f.write(str(int(n)))
		f.write(str(int(y)))
		f.write(str(int(p)))
		f.write(str(int(q)))
		f.close()

	print_key()
	os.system(f'mv key.txt {folder_name}/server/')

def run_scdptopk(folder_name, eps, k):
	os.system(f'cd ./{folder_name}/server && ./scdptopk -4 {eps} -k {k} -r 0 &')
	os.system(f'cd ./{folder_name}/client && ./scdptopk -4 {eps} -k {k} -r 1')

def one_run(eps, k):
	folder_name = f'eps_{eps}_k_{k}'
	copy_file(folder_name)
	copy_key(folder_name)
	run_scdptopk(folder_name, eps, k)

if __name__ == '__main__':
	eps_list = [0.5, 1.0, 2.0]
	k_list = [5, 10, 50]
	os.system('mkdir -p foursqure')
	build_scdptopk()
	for eps in eps_list:
		for k in k_list:
			one_run(eps, k)
	os.system('mv eps_* foursqure')

