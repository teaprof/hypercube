from unittest.mock import Base
import pandas as pd
import json

def convert(input_file_name, output_file_name):
    try:
        print(f"converting {input_file_name} to {output_file_name}")
        with open(input_file_name) as f:
            data = json.load(f)
        
        df = pd.json_normalize(data)
        df.to_excel(output_file_name)
        df.to_html(input_file_name + '.html')
        print(" Done")
    except FileNotFoundError as e:
        print(" input file not found ")
    print("")

if __name__ == '__main__':
    convert('subtasks.json', 'subtasks.xlsx')
    convert('subtasks_results.json', 'subtasks_results.xlsx')
    convert('tasks_results.json', 'tasks_results.xlsx')    