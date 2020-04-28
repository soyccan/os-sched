import sys
import math

time_unit = float(open('timeunit').read())

n = int(sys.argv[1])
cor = sys.argv[2]
out = sys.argv[3]
dmesg = sys.argv[4]

result = []

# get data from program output
for l in open(out):
    name, pid = l.split(' ')
    result.append({
        'pid': int(pid),
        'st': None,
        'en': None,
        'name': name})
result.sort(key=lambda x: x['name'])

# get data from dmesg
ori = 2147483647
for line in open(dmesg):
    pid, st, en = line.split(' ')[-3:]
    for r in result:
        if r['pid'] == int(pid):
            r['st'] = float(st)
            r['en'] = float(en)
            ori = min(ori, float(st), float(en))


# get correct output
cor_a = []
ori_u = 2147483647
for l in open(cor):
    st_u, en_u = map(float, l.split(' '))
    ori_u = min(ori_u, st_u, en_u)
    cor_a.append((st_u, en_u))

# calc accuracy by standard deviation
stdev = 0
for i in range(len(cor_a)):
    result[i]['st'] = (result[i]['st'] - ori) // time_unit
    b = cor_a[i][0] - ori_u
    stdev += (result[i]['st'] - b)**2

    result[i]['en'] = (result[i]['en'] - ori) // time_unit
    b = cor_a[i][1] - ori_u
    stdev += (result[i]['en'] - b)**2

stdev = math.sqrt(stdev / n / 2)
print('----------------')
print(f'{out} {stdev=}')

# print correct output in time unit
print('correct output:')
for i in range(len(cor_a)):
    print(' ', cor_a[i][0]-ori_u, cor_a[i][1]-ori_u)

# print program output in time unit
print('program output:')
for r in result:
    print(' ', r['name'], r['st'], r['en'])
