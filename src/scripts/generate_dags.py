import json, datetime
from airflow import DAG
from airflow.operators import bash

import os 

#subtasks.json should be placed in the same directory as this script
dir_path = os.path.dirname(os.path.realpath(__file__))
json_file=os.path.join(dir_path, "subtasks.json")
#json_file=os.path.join(dir_path, "../build/debug/", "subtasks.json")

with open(json_file) as f:
#with open('subtasks.json') as f:
    data = json.load(f)

with DAG(dag_id="all", start_date=datetime.datetime(2021, 1, 1), schedule="@once") as dag:
    for it in data:
        #it['hash']"
        task = bash.BashOperator(task_id=str(it['taskId']), bash_command = f"cd /opt/airflow/dags && hypercube subtask --id {it['taskId']} --input {json_file}", pool_slots=1)
        #dag<<task
pass
