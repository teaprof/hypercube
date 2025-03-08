from airflow import DAG, Dataset
from airflow.decorators import task
import datetime, json, os
from HypercubeProblem import HypercubeProblem

dims = [1, 2]
mIntervalsPerDim = [10, 20]
nPointsPerCell= [10, 100, 1000, 10000]

outlet_file_name = HypercubeProblem.problems_file_name

@task(outlets=[Dataset(outlet_file_name)])
def generate_problems():
    problems = []
    for d in dims:
        for m in mIntervalsPerDim:
            for n in nPointsPerCell:
                problem = HypercubeProblem(d, m, n)
                problems.append(problem.fileds())
    
    with open(outlet_file_name, "w") as f:
        json.dump(problems, f, indent=2)


with DAG(dag_id="generate_problems", schedule="@once", start_date=datetime.datetime.now()) as dag:
    generate_problems()