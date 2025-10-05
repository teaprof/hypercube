from HypercubeProblem import HypercubeProblem
from airflow import DAG, Dataset
from airflow.operators import bash
from airflow.decorators import task, task_group

import os
import json, datetime

def generateCompressedProblem(problem):
    h = HypercubeProblem.fromFields(problem)
    return [h.dim, h.nIntervalsPerDim, h.nPointsPerCell]

@task
def readProblems():
    try:
        with open(HypercubeProblem.problems_file_name, "r") as file:
            problems = json.load(file)
        return problems
    except FileNotFoundError as err:
        return []

@task
def getSubtasks(problems):
    res = []
    for problem in problems:
        obj = generateCompressedProblem(problem)
        h = HypercubeProblem(*obj)
        subtasks = h.getSubtasks()
        for subtask in subtasks:
            res.append((obj, subtask))
    return res
    
@task.bash
def hypercubeRun(subtask): 
    hargs = subtask[0]
    subtask_idx = subtask[1]
    h = HypercubeProblem(*hargs)    
    run_cmd = h.hypercubeRunSubtaskCmd(subtask_idx)
    return run_cmd
    
@task.bash(outlets=[Dataset(HypercubeProblem.tasks_results_file_name)])
def reduce(dummy, subtask):
    hargs = subtask[0]
    subtask_idx = subtask[1]
    h = HypercubeProblem(*hargs)    
    gather_cmd = h.hypercubeGatherCmd()
    return gather_cmd


#@task_group()
#def solveHypercubeProblem(problem):
#    args = generateObj(problem)
#    subtask_ids = getSubtasks(hargs=args)
#    expanded_values = hypercubeRun.partial(hargs=args).expand(subtask=subtask_ids)
#    reduce(hargs=args).set_upstream(expanded_values)

with DAG(dag_id="solve", schedule=[Dataset(HypercubeProblem.problems_file_name)], start_date=datetime.datetime.now()) as dag:
    #problems = readProblems()
    #for problem in problems:
    #    solveHypercubeProblem(problem)
    problems = readProblems()
    subtasks = getSubtasks(problems)
    subtasks_results = hypercubeRun.expand(subtask=subtasks)
    reduce.partial(dummy=subtasks_results).expand(subtask=subtasks)
        
                
if __name__ == "__main__":
    #dag.test()
    print("Ok")