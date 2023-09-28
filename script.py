import subprocess

def trmm_s(config):
    return f'./trmm_s -s {config["size"]}'

def trmm_t(config):
    return f'./trmm_t -s {config["size"]} -t {config["streams"]}'

def trmm_m(config):
    return f'mpirun -n {config["streams"]} ./trmm_m -s {config["size"]}'

def trmm_mt(config):
    return f'mpirun -n {config["streams"]//2} ./trmm_m -s {config["size"]} -t {2}'

methods = [trmm_t, trmm_m, trmm_mt]
sizes = ['s', 'm', 'l']
streams = [int(2**n) for n in range(1, 6)] 

configs = [{'method': trmm_s, 'size': size} for size in sizes] + \
    [{'method': method, 'size': size, 'streams': stream} 
    for method in methods for size in sizes for stream in streams]
    

perf = "perf stat -x '|' -e duration_time,cache-misses,context-switches"

try:
    file = open("example.txt", "a")
except Exception as e:
    print(e)
    exit()

for config in configs:
    for i in range(10):
        command = f'{perf} {config["method"](config)}'
        
        print(command)

        # output = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

        # print(output.stderr)


file.close()
