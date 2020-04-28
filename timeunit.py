import math

n = 10

# get data from dmesg
lines = open('/var/log/kern.log').readlines()
tm = []
for line in reversed(lines):
    if '[Project1]' in line:
        n -= 1
        st, end = line.split(' ')[-2:]
        tm += [float(end), float(st)]
    if n <= 0:
        break
tm.reverse()

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
