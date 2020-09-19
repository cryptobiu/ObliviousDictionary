import multiprocessing
from multiprocessing import Pool
import math
import os
import uuid
from os import listdir
from os.path import isfile, join
from operator import add
# import numpy
import datetime

BATCH = 500
PATH = "results"
NEW_GROUP_FILE = "totalgroup-m-%d-ratio-%.3f-totalruns-%d-.txt"
NEW_DETAILED_FILE = "total-m-%d-ratio-%.3f-.txt"

def runproc(cmd):
    print("runproc CMD=%s"%cmd)
    os.system(cmd)


def check(n_runs, m, ratio):
    n_cpus = multiprocessing.cpu_count()
    n_processes = n_cpus
    n_runs_per_proc = math.ceil(n_runs/n_processes)
    batch = n_runs_per_proc if n_runs_per_proc<BATCH else BATCH

    dataset = []
    for i in range(n_processes):
        identifier = uuid.uuid1()
        command = "./obliviousDictionary -partyID 0 -internalIterationsNumber %d -hashSize %d -partiesFile Parties.txt -reportStatistics 1 -version 3Tables -tool poly -batchSize %d -processId %d -tableRatio %.2f"%(n_runs_per_proc, m, batch, identifier.int, ratio)
        print(command)
        dataset.append(command)
    
    start = datetime.datetime.now()
    with Pool(processes=n_processes) as pool:
        result = pool.map(runproc, dataset, 1)
    end = datetime.datetime.now()
    difftime = end-start
    millis = difftime.seconds*1000+difftime.microseconds/1000
    print("%s\nfinished."%("*"*10))
    print("Time=%d seconds, or %d milliseconds per run\n%s"%(difftime.seconds, float(millis)/float(n_runs), "*"*10 ))



def aggregate_file(fname):
    file1 = open(join(PATH,fname), 'r') 
    lines = file1.readlines()
    x_batch_fails = [0,0,0,0,0,0] #total number of failures per batch for [0.5,1,2,3,4,5](log m)
    num_batches = 0
    for l in lines[1:]: #the first line is only a header
        num_batches += 1
        failures = [int(x) for x in l.split(',')]
        x_batch_fails = list( map(add, failures, x_batch_fails) )
        # print(str(failures))
    print("batch total:\n%s"%str(x_batch_fails))
    return num_batches, x_batch_fails



def process_results(m, ratio):
    print("processing results")

    onlyfiles = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    
    #the group files first
    x_file_fails = [0,0,0,0,0,0] #total number of failures per file for [0.5,1,2,3,4,5](log m)
    total_runs = 0
    for f in onlyfiles:
        args = f.split("-")
        if "group"==args[0] and m==int(args[3]) and ratio==float(args[7]):
            print("group file: %s"%str(args))
            batch = int(args[5])
            num_batches, x_batch_fails = aggregate_file(f)
            x_file_fails = list( map(add, x_file_fails, x_batch_fails) )
            total_runs += batch*num_batches
    newgroup_fname = join(PATH, NEW_GROUP_FILE%(m,ratio,total_runs))
    with open(newgroup_fname,'w') as newgroupfile:
        newgroupfile.write(', '.join([str(x) for x in x_file_fails] ))
        newgroupfile.close()
        print("written to %s"%newgroup_fname)
    print("files total:\n%s"%str(x_file_fails))
    print("total runs: %d"%total_runs)

        
    #the raw files now
    coresizes = {}
    for f in onlyfiles:
        args = f.split("-")
        if "ungroup"==args[0] and m==int(args[3]) and ratio==float(args[7]):
            print("ungroup file: %s"%str(args))
            file1 = open(join(PATH,f), 'r')
            while True:
                l = file1.readline()
                if not l: 
                    break
                size = int(l)
                coresizes[size] = coresizes[size]+1 if size in coresizes else 1
            file1.close()


    print("2coresizes:\n%s"%(str(coresizes)))
    
    newdetails_fname = join(PATH, NEW_DETAILED_FILE%(m,ratio))
    with open(newdetails_fname,'w') as newdetiledfile:
        for i in sorted(coresizes.keys()):
            newdetiledfile.write("%d,%d\n"%(i, coresizes[i]))
        newdetiledfile.close()
        print("written to %s"%newdetails_fname)



def get_failure_ratio(m, ratio):
    onlyfiles = [f for f in listdir(PATH) if isfile(join(PATH, f))]
    for f in onlyfiles:
        args = f.split("-")
        if "totalgroup"==args[0] and m==int(args[2]) and ratio==float(args[4]):
            total_runs = args[6]
            file1 = open(join(PATH,f), 'r')
            l = file1.readline()
            failures = l.split(',')
            file1.close()
            return float(failures[1])/float(total_runs)
            

# def find_best_ratio(m):

#     best_ratio = 1.25

#     possibilities = [round(i,2) for i in numpy.arange(1.25,1.51,0.01)]
#     checked = [0 for i in possibilities]
#     print(possibilities)
#     print(checked)

#     print(possibilities.index(1.25))
#     print(possibilities.index(1.49))





if __name__ == "__main__":
    n_runs = 2000
    m = 8000
    ratio = 1.25
    check(n_runs, m, ratio)
    process_results(m, ratio)
    fp = get_failure_ratio(m, ratio)
    print("fp = %f"%fp)
    # find_best_ratio(m)