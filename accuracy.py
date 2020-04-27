import sys
import math

time_unit = float(open('timeunit').read())

filename = sys.argv[1] + '.txt'
cor = open('corr_out/' + filename)
out = open('out/' + filename)
n = len(cor.readlines())
m = n

result = []

# get data from dmesg
for line in reversed(open('/var/log/kern.log').readlines()):
    if '[Project1]' in line:
        n -= 1
        pid, st, en = line.split(' ')[-3:]
        result.append({
            'en': float(en),
            'st': float(st),
            'pid': int(pid),
            'name': None})
    if n <= 0:
        break
ori = min(sum(([x['st'], x['en']] for x in result), start=[]))

# get data from program output
for l in out.readlines():
    name, pid = l.split(' ')
    for r in result:
        if r['pid'] == int(pid):
            r['name'] = name
result.sort(key=lambda x: x['name'])

# calc accuracy by sum of squared error
cor.seek(0, 0)
ori_u = -1
sse = 0
for i, l in enumerate(cor.readlines()):
    st_u, en_u = map(float, l.split(' '))
    if ori_u == -1:
        ori_u = st_u
    result[i]['st'] = (result[i]['st'] - ori) // time_unit
    result[i]['en'] = (result[i]['en'] - ori) // time_unit
    sse += (result[i]['st'] - st_u + ori_u)**2
    sse += (result[i]['en'] - en_u + ori_u)**2
sse /= m*2
sse = math.sqrt(sse)
print('----------------')
print(f'{sys.argv[1]} {sse=}')

# print correct output in time unit
print('correct output:')
cor.seek(0, 0)
ori = -1
for l in cor.readlines():
    st, en = map(int, l.split(' '))
    if ori == -1:
        ori = st
    print(' ', st-ori, en-ori)

# print program output in time unit
print('program output:')
for r in result:
    print(' ', r['name'], r['st'], r['en'])
