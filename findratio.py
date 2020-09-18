import multiprocessing
from multiprocessing import Pool
import math
import os
import uuid 

BATCH = 10000

def runproc(cmd):
    print("runproc CMD=%s"%cmd)
    os.system(cmd)


def check(n_runs, m, ratio):
    n_cpus = multiprocessing.cpu_count()
    n_processes = n_cpus
    n_runs_per_proc = math.ceil(n_runs/n_processes)

    dataset = []
    for i in range(n_processes):
        identifier = uuid.uuid1()
        command = "./obliviousDictionary -partyID 0 -internalIterationsNumber %d -hashSize %d -partiesFile Parties.txt -reportStatistics 1 -version 3Tables -tool poly -batchSize %d -processId %d -tableRatio %.2f"%(n_runs_per_proc, m, BATCH, identifier.int, ratio)
        print(command)
        dataset.append(command)
    
    with Pool(processes=n_processes) as pool:
        result = pool.map(runproc, dataset, 1)

    print("finished")

    

def process_results(m, ratio):
    pass


if __name__ == "__main__":
    check(2000, 8000, 1.31)