import json, datetime
from airflow import DAG
from airflow.operators import bash

with open('/opt/airflow/dags/subtasks.json') as f:
#with open('subtasks.json') as f:
    data = json.load(f)

for it in data:
    with DAG(dag_id=str(it['hash']), start_date=datetime.datetime(2021, 1, 1), schedule="@once"):
        bash.BashOperator(task_id="bashcmdtea", bash_command = f"echo {it['hash']}")
pass
