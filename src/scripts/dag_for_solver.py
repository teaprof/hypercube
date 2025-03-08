from HypercubeProblem import HypercubeProblem
from airflow import DAG, Dataset
from airflow.operators import bash
from airflow.decorators import task, task_group

import os
import json, datetime

def readProblems():
    try:
        with open(HypercubeProblem.problems_file_name, "r") as file:
            problems = json.load(file)
        return problems
    except FileNotFoundError as err:
        return []

@task
def getSubtasks(hargs):
    h = HypercubeProblem(*hargs)
    res = h.getSubtasks()
    return res
    
@task.bash
def hypercubeRun(hargs, subtask: int):
    h = HypercubeProblem(*hargs)
    run_cmd = h.hypercubeRunSubtaskCmd(subtask)
    return run_cmd
    
@task.bash(outlets=[Dataset(HypercubeProblem.tasks_results_file_name)])
def reduce(hargs):
    h = HypercubeProblem(*hargs)
    gather_cmd = h.hypercubeGatherCmd()
    return gather_cmd

@task
def generateObj(problem):
    h = HypercubeProblem.fromFields(problem)
    return [h.dim, h.nIntervalsPerDim, h.nPointsPerCell]

@task_group()
def solveHypercubeProblem(problem):
    args = generateObj(problem)
    subtask_ids = getSubtasks(hargs=args)
    expanded_values = hypercubeRun.partial(hargs=args).expand(subtask=subtask_ids)
    reduce(hargs=args).set_upstream(expanded_values)

with DAG(dag_id="solve", schedule=[Dataset(HypercubeProblem.problems_file_name)], start_date=datetime.datetime.now()) as dag:
    problems = readProblems()
    for problem in problems:
        solveHypercubeProblem(problem)
                
if __name__ == "__main__":
    #dag.test()
    print("Ok")