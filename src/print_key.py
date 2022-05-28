from gm import generate_key

key_A = generate_key()
n, y = key_A['pub']
p, q = key_A['priv']
print(n)
print(y)
print(p)
print(q)
