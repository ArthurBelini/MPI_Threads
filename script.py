import subprocess

def trmm_s(config):
    return f'./trmm_s -s {config["size"]}'

def trmm_t(config):
    return f'./trmm_t -s {config["size"]} -t {config["streams"]}'

def trmm_m(config):
    return f'mpirun -n {config["streams"]} ./trmm_m -s {config["size"]}'

def trmm_mt(config):
    return f'mpirun -n {config["streams"]//2} ./trmm_mt -s {config["size"]} -t {2}'

methods = [trmm_t, trmm_m, trmm_mt]
sizes = ['s', 'm', 'l']
streams = [int(2**n) for n in range(1, 6)] 
metrics = ['duration_time', 'cache-misses', 'context-switches'] 

configs = [{'method': trmm_s, 'size': size} for size in sizes] + \
    [{'method': method, 'size': size, 'streams': stream} 
    for method in methods for size in sizes for stream in streams]
    

perf = f'perf stat -x "|" -e {",".join(metrics)}'

try:
    file = open("perf.txt", "a")
except Exception as e:
    print(e)
    exit()

its = 2
s_means = {size: 0 for size in sizes}
for config in configs:
    command_run = config["method"](config)
    command_perf = f'{perf} {command_run}'
    cur_size = config['size']
    mean = 0

    file.write(f'{command_run}:\n')

    for i in range(its):
        lines = subprocess.run(command_perf, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True).stderr
        lines = lines.strip('\n')
        lines = lines.split('\n')
        lines = [line.strip('|') for line in lines]
        lines = [line.split('|') for line in lines]

        mean += int(lines[0][0])

        lines = [f'{line[2]} = {line[0]}' for line in lines]

        file.write(f'{i}. {", ".join(lines)}\n')

    mean /= its

    file.write(f'mean = {mean}\n')

    if config['method'] == trmm_s:
        s_means[cur_size] = mean
    else:
        speedup =  s_means[cur_size] / mean
        efficiency = speedup / config['streams']

        file.write(f'speedup = {speedup}\n')
        file.write(f'efficiency = {efficiency}\n')

    file.write('\n')

file.close()
