#! /bin/python
"""!
@file schedulability_test.py
@ingroup utils
@author Mattia Nicolella
@brief General procedures for a schedulability test

This script contains procedures to generate data for a schedulability test done using benchmarks in this repository.

__NOTE:__ This script might require administrative privileges (to change it scheduling policy and move processes in other cores).

Dependencies:
- Python 3.5+
- lscpu
- grep
- sort
- wc
- ps
- taskset
- mkdir
"""

import argparse
import subprocess
import os
import csv

def sched_test(bmark,bmark_args,worst_case,util_inc,tasks_num,output,prefix,postfix):
    """!
    @brief schedulability test for a single benchmark.
    @param[in] bmark The benchmark executable.
    @param[in] bmark_args Arguments for the target benchmark.
    @param[in] worst_case Worst case execution time of the target benchmark.
    @param[in] util_inc Increase of the utilization (percentage) at every step of the test.
    @param[in] tasks_num Number of tasks to execute at each step of the test.
    @param[in] output Output folder of for the data files.
    @param[in] prefix Prefix to prepend to all generated files.
    @param[in] postfix Postfi to append to all generated files.
    @details
    The test starts with a deadline equal to the worst case execution time, the this deadline will progressively decrease by a percentage (given by the user) until it reaches 0.
    For each of these steps the task execution will be aggregated in a mean and the number of tasks that did complete without missing the deadline are recorded, along with the number
    of tasks launched per step.

    The benchmark is instructed to log data in a set of files called: `sched_test_x.csv` where `x` is the expected utilization percentage, while the number of scheduled processes at each step will be recorded in `sched_test_res.csv`.
    @returns `0` on success, `-1` on error.
    """
    deadline=worst_case
    utilization=1-(deadline/worst_case)
    try:
        res_file=open(os.path.join(output,prefix+"sched_test_res"+postfix+".csv"),"w")
    except Exception as e:
        print("Cannot open file for storing schedulability test results ",e)
        return -1
    writer=csv.writer(res_file)
    writer.writerow(["utilization","mean_utilization","successfully_scheduled","total_started"])
    print(f"\n\nStarting schedulability test for {bmark}")
    while deadline >= 1e-9:
        sum_utilization=0
        print(f"\ntest  with {utilization*100}% utilization, deadline: {deadline} seconds")
        log_fname=os.path.join(output,f"{prefix}sched_test_{utilization}{postfix}.csv")
        try:
            subprocess.run([bmark,"-d",str(deadline),"-p",str(deadline),"-l","2","-c","0","-t",str(tasks_num),"-o",log_fname,"-b"]+bmark_args)
        except Exception as e:
            print("Error during schedulability test ",e)
            return -1
        scheduled=0
        started=0
        try:
            log_file=open(log_fname)
        except Exception as e:
            print("Cannot open benchmark result file ", e)
            return -1
        reader=csv.reader(log_file)
        next(reader)
        for row in reader:
            started+=1
            if int(row[10]) == 1:
                scheduled+=1
            sum_utilization+=float(row[11])
        log_file.close()
        print(f"successfully scheduled {scheduled} over {started} tasks ({scheduled/started*100}%), mean utilization: {sum_utilization/started}")
        writer.writerow([str(utilization),str(sum_utilization/started),str(scheduled),str(started)])
        deadline=deadline-(util_inc*worst_case)
        utilization=1-(deadline/worst_case)
    res_file.close()
    return 0

def start_interfering(deadline,bmarks,bmarks_args,num_cpus):
    """!
    @brief Launches interfering benchmarks
    @param[in] deadline The deadline (and period) to give to the interfering benchmarks.
    @param[in] bmarks The list of interfering benchmarks executables.
    @param[in] bmarks_args The list of arguments for each interfering benchmark.
    @param[in] num_cpus The number of available physical cores.
    @details
    The interfering benchmarks can be executed on all the available cores, excluding the first, the decision of which benchmark to place on which core is left to the scheduler.
    The benchmarks will continue to be executed until they receive a `SIGINT`.
    @returns The list of process id associated to the spawned benchmarks or `-1` on error.
    """
    cpu_range="1"
    for i in range(2,num_cpus):
        cpu_range+=","+str(i)
    bmark_processes=[]
    for i in range(0,len(bmarks)):
        try:
            bmark_processes.append(subprocess.Popen([bmarks[i],"-d",str(deadline),"-p",str(deadline),"-l","1","-c",cpu_range]+bmarks_args[i]))
        except Exception as e:
            print("Error while starting interfering benchmarks")
            stop_interfering(bmark_processes)
            return -1
    return bmark_processes

def stop_interfering(processes):
    """!
    @brief Stops the running interfering benchmarks.
    @param[in] processes The list of interfering benchmark that are in execution.
    @details
    The interfering benchmarks are stopped with a `SIGINT`.
    """
    for process in processes:
        process.send_signal(signal.SIGINT)


def worst_case_exec_test(bmark,bmark_args,worst_case_tests,output,prefix,postfix):
    """!
    @brief Finds the worst case execution time using only the first core.
    @param[in] bmark The target benchmark.
    @param[in] bmark_args The arguments for the target benchmark.
    @param[in] worst_case_test The number of tests to execute for detecting the worst case scenario-
    @param[in] output The output folder for the generated files.
    @param[in] prefix The prefix to prepend to the generated files.
    @param[in] postfix The postfix to prepend to the generated files.
    @details
    The worst case execution time will be the longest time a single task has run without missing the deadline.
    To do so a number of tasks (`worst_case_tests`) is run and if at least one misses the deadline then the deadline be increased and the test restarted.
    After the all tasks have completed their execution, the output file will be scanned to find the maximum execution time.

    The maximum execution time will be saved in clock cycles and in seconds.

    The found worst case execution time will be exported into a file called `worst_case_exec.csv`
    @returns The found worst case execution time in seconds or -1 in case of error.
    """
    deadline=0.001
    fails_count=1
    try:
        worst_file=open(os.path.join(output,prefix+"worst_case_exec"+postfix+".csv"))
        prev_reader=csv.reader(worst_file)
        next(prev_reader)
        row=next(prev_reader)
        worst=int(row[0])
        worst_time=float(row[1])
        worst_file.close()
        print(f"read {worst} clock cycles : {worst_time} seconds")
    except Exception as e:
        print("worst_case_exec.csv not found, creating it",e)
        worst=0
        worst_time=0
    worst_file=open(os.path.join(output,prefix+"worst_case_exec"+postfix+".csv"),"w")
    writer=csv.writer(worst_file)
    writer.writerow(["worst_in_clock","worst_in_seconds"])
    print(f"\nstarting worst case execution test for {bmark}")
    while fails_count>0 :
        print(f"deadline value: {deadline}")
        fails_count=0
        subprocess.run([bmark,"-d",str(deadline),"-p",str(deadline),"-l","2","-c","0","-t",str(worst_case_tests),"-o",os.path.join(output,prefix+"worst_case_runtime_test"+postfix+".csv"),"-b"]+bmark_args)
        try:
            test_file=open(os.path.join(output,prefix+"worst_case_runtime_test"+postfix+".csv"))
        except Exception as e:
            print("Error opening worst case execution test report file",e)
            return -1
        reader=csv.reader(test_file,delimiter=",")
        #skip the header
        next(reader)
        for row in reader:
            if int(row[10])==0:
                fails_count+=1
            if float(row[9]) > worst_time:
                worst_time=float(row[9])
                worst=int(row[4])
        test_file.close()
        if fails_count>0:
            print(f"{fails_count} benchmark failed, increasing deadline")
            deadline *=10
    print(f"done, test results:{worst} clock cycles {worst_time} seconds\n")
    writer.writerow([worst,worst_time])
    worst_file.close()
    return worst_time

def detect_cores():
    """!
    @brief Detect physical and logical cores.
    @returns The number of available physical and logical cores in the machine, as a tuple (phys,logic). -1 is given on error.
    """
    #get the number of available cores
    try:
        phys_cores=int(subprocess.check_output("lscpu -b -p=Core,Socket | grep -v '^#' | sort -u | wc -l",text=True,shell=True))
    except Exception as e:
        print("Error during retrieval of number physical of cores: ",e)
        return -1
    try:
        logic_cores=int(subprocess.check_output("grep -c ^processor /proc/cpuinfo",text=True,shell=True))
    except Exception as e:
        print("Error during retrieval of number logical of cores: ",e)
        return -1
    return (phys_cores,logic_cores)



def move_processes(corelist):
    """!
    @brief Move processes according to the provided corelist.
    @param[in] corelist The list of cores (as a `taskset` compatible string) on which processes are allowed to run.
    @details
    Moves all the existing processes (assumed to not be part of the testing set) according to the provided corelist, to avoid interference as much as possible.

    __NOTE__: Even as root, not all processes can be moved between cores. This types of errors will be silently ignored.
    """
    print(f"moving processes on corelist {corelist}")
    #get the number of processes
    try:
        task_list=subprocess.check_output(["ps","-e","-o","pid"],text=True)
    except Exception as e:
        print("Error during retrieval of process list: ",e)
        return -1
    #after having the pid list, we can use tasklist to set the cpu affinity to everything but core 0
    for pid in task_list.split('\n'):
        pid=pid.strip()
        if pid != "PID" and pid != ' ' and pid!='':
            try:
                subprocess.check_output(["taskset","-cp",corelist,pid],stderr=subprocess.DEVNULL)
            except Exception:
                pass
    print("done")

def handle_bmark_list(bmark_list):
    """!
    @brief Split the list of benchmark in executable,arguments tuples.
    @param[in] bmark_list The list of benchmark in the `"executable:arg1,arg2,..."` format.
    @returns a list of benchmark in the `["executable","arg1 arg2 ..."]` format.
    """
    new_list=[]
    for bmark in bmark_list:
        tmp=bmark.split(':')
        new_list.append((tmp[0],tmp[1].split(',')))
    return new_list

def main():
    """!
    @brief The script main function
    @details
    This function will handle the parameters parsing and the test execution by:
    - detecting the number of cores;
    - moving all processes to the last physical core;
    - changing the scheduling policy for this process to `SCHED_FIFO` with priority 1;
    - detect the worst execution time (WCET) for each target benchmark;
    - starting the interfering benchmarks if supplied;
    - executing the schedulability test for each target benchmark;
    - stopping the interfering benchmarks.
    - restore the default core affinity for all the other processes.
    @returns `0` on success, `-1` on error.
    """
    # set up the argument parser
    parser = argparse.ArgumentParser(description='A script to perform a schedulability test')
    parser.add_argument('-u','--utilization-increase', metavar='utilization', type=float,help='How much the utilization should increase at each test step.',default=0.1,choices=map(lambda x: x/100.0, range(1, 100)),required=False,dest='util_inc')

    parser.add_argument('-t','--tasks-number', metavar='tasks-num', type=int,help='The number of tasks to be executed for each test step.',default=100,required=False,dest='tasks_num')

    parser.add_argument('-wt','--worst-case-tests', metavar='tests-num', type=int,help='The number of tasks to be executed when searching for the worst case runtime.',default=1000,required=False,dest='worst_case_tests')

    parser.add_argument('-i','--interfering-bmark', metavar='bmark-exec:arg1,arg2,...',nargs='+',type=str, help='A list of interfering benchmarks executables, with their arguments.  This option can be specified multiple times.',default=[],required=False,dest='interfering')

    parser.add_argument('-b','--bmarks', metavar='bmark-exec:arg1,arg2,...', nargs='+', type=str,help='A list of  benchmarks executables, with their arguments. A schedulability test will be performed on each of these benchmarks.\n This option can be specified multiple times.',required=True,dest='benchmarks')

    parser.add_argument('-o','--output',metavar='path',nargs='+',type=str,help='The location where all the generated output files will be located. This option can be repeated for each target benchmark.',required=False,default=[],dest='output')

    parser.add_argument('-pre','--prefix',metavar='prefix',type=str,help='A prefix to set on the generated files',required=False,default='',dest='prefix')

    parser.add_argument('-post','--postfix',metavar='postfix',type=str,help='A postfix to set on the generated files',required=False,default='',dest='postfix')
    #parse arguments
    args=parser.parse_args()
    #adjust the list of benchmarks and interfering benchmarks
    args.benchmarks=handle_bmark_list(args.benchmarks)
    args.interfering=handle_bmark_list(args.interfering)
    #we detect the physical cores
    cores=detect_cores()
    if cores == -1:
        return cores
    #we move all processes to the last core
    move_processes(f"{cores[0]-1}")
    #before starting the execution we set the scheduling policy for us and all out child processes
    try:
        subprocess.run(["chrt","-f","-p","-a","1",str(os.getpid())])
    except Exception as e:
        print("Error changing the scheduler policy ",e)
        #we restore the normal execution of the tasks
        move_processes(f"0-{cores[1]-1}")
        return -1

    output_len=len(args.output)
    for i in range(0,len(args.benchmarks)):
        if output_len ==0:
            args.output.append(os.path.dirname(args.benchmarks[i][0]))
        elif output_len == len(args.benchmarks):
            subprocess.run(["mkdir","-p",args.output[i]])
        else:
            print("Error: A single folder has been specified as output path for more than one target benchmark, this WILL overwrite data from all but the last target benchmark.")
            return -1
    #get the list of worst case runtimes
    worst_runtimes=[]
    for i in range(0,len(args.benchmarks)):
        WCET=worst_case_exec_test(args.benchmarks[i][0],args.benchmarks[i][1],args.worst_case_tests,args.output[i],args.prefix,args.postfix)
        if WCET < 0:
            #we restore the normal execution of the tasks
            move_processes(f"0-{cores[1]-1}")
            return WCET
        worst_runtimes.append(WCET)
    # if the user requested it, we start the interfering benchmarks
    if args.interfering != []:
        int_processes=start_interfering(min(worst_runtimes),args.interfering[0],args.interfering[1],cores[0])
        if int_processes != []:
            #we restore the normal execution of the tasks
            move_processes(f"0-{cores[1]-1}")
            return -1
    # we start the schedulability test for each benchmark
    for i in range(0,len(args.benchmarks)):
       res=sched_test(args.benchmarks[i][0],args.benchmarks[i][1],worst_runtimes[i],args.util_inc,args.tasks_num,args.output[i],args.prefix,args.postfix)
       if res <0:
           break;
    #we stop the interfering benchmarks if necessary
    if args.interfering != [] and int_processes!=[]:
        stop_interfering(int_processes)
    #we restore the normal execution of the tasks
    move_processes(f"0-{cores[1]-1}")
    return res;

# Execute the main function when this script is not a module
if __name__=="__main__":
    main()
