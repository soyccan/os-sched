import math, sys

n = 10

out = sys.argv[1]
dmesg = sys.argv[2]

pids = []

# get data from program output
for l in open(out):
    name, pid = l.split(' ')[-2:]
    pids.append(int(pid))

# get data from dmesg
tm = []
for line in open(dmesg):
    pid, st, en = line.split(' ')[-3:]
    if int(pid) in pids:
        tm += [float(st), float(en)]

# calc time unit
diff = []
for i in range(len(tm)-1):
    diff.append(tm[i+1] - tm[i])
print('\n'.join(map(str, diff)))
mu = sum(diff) / len(diff)
print('avg =', mu)
print('stdev = ', math.sqrt(sum((x - mu)**2 for x in diff) / len(diff)))
print('time unit =', mu / 500)

open('timeunit', 'w').write(str(mu / 500))
