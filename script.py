import subprocess

def trmm_s(config):
    return f'./trmm_s -s {config["size"]}'

def trmm_t(config):
    return f'./trmm_t -s {config["size"]} -t {config["streams"]}'

def trmm_m(config):
    return f'mpirun -n {config["streams"]} ./trmm_m -s {config["size"]}'

def trmm_mt(config):
    return f'mpirun -n {config["streams"]//2} ./trmm_mt -s {config["size"]} -t {2}'

# methods = [trmm_t, trmm_m, trmm_mt]
methods = [trmm_t]
sizes = ['s']
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

plot_types = ['speedup', 'efficiency']
its = 2
s_means = {size: 0 for size in sizes}
plots = {plot: {method: {size: [] for size in sizes} for method in methods} for plot in plot_types}
speedups = plots['speedup']
efficiencies = plots['efficiency']
for config in configs:
    cur_method = config['method']
    cur_size = config['size']
    command_run = cur_method(config)
    command_perf = f'{perf} {command_run}'
    mean = 0

    file.write(f'{command_run}:\n')

    for i in range(its):
        print(command_perf)
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

    if cur_method == trmm_s:
        s_means[cur_size] = mean
    else:
        cur_streams = config['streams']
        cur_speedup = speedups[cur_method]
        cur_efficiency = efficiencies[cur_method]

        speedup =  s_means[cur_size] / mean
        efficiency = speedup / cur_streams

        cur_speedup[cur_size].append((cur_streams, speedup))
        cur_efficiency[cur_size].append((cur_streams, efficiency))

        file.write(f'speedup = {speedup}\n')
        file.write(f'efficiency = {efficiency}\n')

    file.write('\n')

for plot in plot_types:
    cur_plot = plots[plot]

    file.write(f'{plot}:\n')

    for method in methods:
        cur_method = cur_plot[method]

        file.write(f'{method.__name__}:\n')

        for size in sizes:
            file.write(f'{size}: {cur_method[size]}\n')

    file.write('\n')

file.close()
