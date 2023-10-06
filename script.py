import subprocess
from prettytable import PrettyTable

# Inicialização
def trmm_s(config):
    return f'./trmm_s -s {config["size"]}'

def trmm_t(config):
    return f'./trmm_t -s {config["size"]} -t {config["streams"]}'

def trmm_m(config):
    return f'mpirun -n {config["streams"]} ./trmm_m -s {config["size"]}'

def trmm_mt(config):
    return f'mpirun -n {config["streams"]//2} ./trmm_mt -s {config["size"]} -t {2}'

p_methods = [trmm_t, trmm_m, trmm_mt]
# p_methods = [trmm_t, trmm_m]
all_methods = [trmm_s] + p_methods
sizes = ['s', 'm', 'l']
# sizes = ['s', 'm']
p_streams = [int(2**n) for n in range(1, 6)] 
all_streams = [1] + p_streams
metrics = ['duration_time', 'cache-misses', 'context-switches'] 

configs = [{'method': trmm_s, 'size': size, 'streams': 1} for size in sizes] + \
    [{'method': method, 'size': size, 'streams': stream} 
    for method in p_methods for size in sizes for stream in p_streams]

perf = f'perf stat -x "|" -e {",".join(metrics)}'

try:
    file = open("perf.txt", "a")
except Exception as e:
    print(e)
    exit()

plot_types = ['speedup', 'efficiency']
its = 2
means = {method: {stream_count: {size: {metric: 0 for metric in metrics} for size in sizes} for stream_count in all_streams} for method in all_methods}
plots = {plot: {method: {size: [] for size in sizes} for method in p_methods} for plot in plot_types}
speedups = plots['speedup']
efficiencies = plots['efficiency']
tables = {metric: {stream_count: PrettyTable(['size'] + [method.__name__ for method in all_methods]) for stream_count in p_streams} for metric in metrics if metric != 'duration_time'}

# Execução e coleta de dados das configurações
for config in configs:
    cur_method = config['method']
    cur_size = config['size']
    cur_streams = config['streams']
    cur_means = means[cur_method][cur_streams][cur_size]
    command_run = cur_method(config)
    command_perf = f'{perf} {command_run}'

    file.write(f'{command_run}:\n')

    for i in range(its):
        print(command_perf)
        lines = subprocess.run(command_perf, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True).stderr
        lines = lines.strip('\n')
        lines = lines.split('\n')
        lines = [line.strip('|') for line in lines]
        lines = [line.split('|') for line in lines]

        for j, metric in enumerate(metrics):
            cur_means[metric] += int(lines[j][0]) / its

        lines = [f'{line[2]} = {line[0]}' for line in lines]

        file.write(f'{i}. {", ".join(lines)}\n')

    file.write(f'metrics means:\n')
    for metric in metrics:
        file.write(f'{metric} = {cur_means[metric]}\n')

    if cur_method != trmm_s:
        cur_speedup = speedups[cur_method]
        cur_efficiency = efficiencies[cur_method]
        cur_s_means = means[trmm_s][1][cur_size]

        speedup =  cur_s_means['duration_time'] / cur_means['duration_time']
        efficiency = speedup / cur_streams

        cur_speedup[cur_size].append((cur_streams, speedup))
        cur_efficiency[cur_size].append((cur_streams, efficiency))

        file.write(f'speedup = {speedup}\n')
        file.write(f'efficiency = {efficiency}\n')

    file.write('\n')

# Cálculo dos pontos dos gráficos
for plot in plot_types:
    cur_plot = plots[plot]

    file.write(f'{plot}:\n')

    for method in p_methods:
        cur_method = cur_plot[method]

        file.write(f'{method.__name__}:\n')

        for size in sizes:
            file.write(f'{size}: {cur_method[size]}\n')

    file.write('\n')

# Cálculo das tabelas
for metric in [m for m in metrics if m != 'duration_time']:
    cur_table_metric = tables[metric]

    for stream_count in p_streams:
        cur_trable = cur_table_metric[stream_count]

        for size in sizes:
            cur_trable.add_row([size] + [means[method][stream_count][size][metric] for method in all_methods])

        file.write(f'Tabela de {metric} com {stream_count} fluxos:\n {cur_trable}\n\n')

file.close()

# Teste de resultados: ./trmm_s -s t -p -c ; ./trmm_t -s t -p -c -t 2 ; mpirun -n 2 ./trmm_m -s t -p -c ; mpirun -n 2 ./trmm_mt -s t -t 2 -p -c
