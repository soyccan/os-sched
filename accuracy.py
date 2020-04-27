import sys
import math

time_unit = float(open('timeunit').read())

filename = sys.argv[1] + '.txt'
cor = open('corr_out.1/' + filename)
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
for r in result:
    if r['name'] == None:
        print('not found: {}'.format(r['pid']))
        print('result:', result)
result.sort(key=lambda x: x['name'])

# get correct output
cor.seek(0,0)
cor_a = []
ori_u = 2147483647
for i, l in enumerate(cor.readlines()):
    st_u, en_u = map(float, l.split(' '))
    ori_u = min(ori_u, st_u, en_u)
    cor_a.append((st_u, en_u))

# calc accuracy by sum of squared error
sse = 0
for i in range(len(cor_a)):
    result[i]['st'] = (result[i]['st'] - ori) // time_unit
    result[i]['en'] = (result[i]['en'] - ori) // time_unit
    sse += (result[i]['st'] - cor_a[i][0] + ori_u)**2
    sse += (result[i]['en'] - cor_a[i][1] + ori_u)**2
sse /= m*2
sse = math.sqrt(sse)
print('----------------')
print(f'{sys.argv[1]} {sse=}')

# print correct output in time unit
print('correct output:')
for i in range(len(cor_a)):
    print(' ', cor_a[i][0]-ori_u, cor_a[i][1]-ori_u)

# print program output in time unit
print('program output:')
for r in result:
    print(' ', r['name'], r['st'], r['en'])
