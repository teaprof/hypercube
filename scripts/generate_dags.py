import json, datetime
from airflow import DAG
from airflow.operators import bash

import os 
dir_path = os.path.dirname(os.path.realpath(__file__))
json_file=os.path.join(dir_path, "subtasks.json")
#json_file=os.path.join(dir_path, "../build/debug/", "subtasks.json")

with open(json_file) as f:
#with open('subtasks.json') as f:
    data = json.load(f)

for it in data:
    with DAG(dag_id=str(it['taskId']), start_date=datetime.datetime(2021, 1, 1), schedule="@once"):
        #it['hash']
        bash.BashOperator(task_id="bashcmdtea", bash_command = f"cd /opt/airflow/dags && hypercube subtask --id {it['taskId']}")
pass
