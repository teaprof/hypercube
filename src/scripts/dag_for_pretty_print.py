from HypercubeProblem import HypercubeProblem
from airflow import DAG, Dataset
from airflow.decorators import task
import pandas as pd
import json, os, datetime
from pathlib import PurePath

results_file_name = HypercubeProblem.tasks_results_file_name
table_file_name_xls = PurePath(results_file_name).with_suffix(".xlsx").as_posix()
table_file_name_csv = PurePath(results_file_name).with_suffix(".csv").as_posix()
table_file_name_html = PurePath(results_file_name).with_suffix(".html").as_posix()

@task(outlets=[Dataset(table_file_name_xls), Dataset(table_file_name_csv), Dataset(table_file_name_html)])
def convert(input_file_name):
    try:
        print(f"converting {input_file_name} to xls, html and csv formats")
        with open(input_file_name) as f:
            data = json.load(f)
        
        df = pd.json_normalize(data["tasks"])
        df.to_excel(table_file_name_xls)
        df.to_html(table_file_name_html)
        df.to_csv(table_file_name_csv)
        print(" Done")
    except FileNotFoundError as e:
        print(" input file not found ")
    print("")

with DAG(dag_id="pretty_print", schedule=[Dataset(results_file_name)], start_date=datetime.datetime.now()) as dag:
    convert(results_file_name)
