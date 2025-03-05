import json, datetime
from airflow import DAG
from airflow.operators import bash

import os , subprocess

#subtasks.json should be placed in the same directory as this script
dir_path = os.path.dirname(os.path.realpath(__file__))
json_file=os.path.join(dir_path, "subtasks.json")
#json_file=os.path.join(dir_path, "../build/debug/", "subtasks.json")

    
dims = [1, 2]
mIntervalsPerDim = [10, 20]
nPointsPerCell= [10, 100, 1000, 10000]

class HypercubeProblem:
    def __init__(self, dim, nIntervalsPerCell, nPointsPerCell):
        self.dim = dim
        self.nIntervalsPerCell = nIntervalsPerCell
        self.nPointsPerCell = nPointsPerCell
        
    @property
    def hypercubeExe(self):
        #return "./bin/hypercube"
        return "hypercube"
        
    @property
    def _fileUniqSuffix(self):
        return f"d{self.dim}m{self.nIntervalsPerCell}nps{self.nPointsPerCell}"
    
    @property
    def subtasksFileName(self):            
        return f"subtasks{self._fileUniqSuffix}.json"

    @property
    def subtasksResultsFileName(self):            
        return f"subtasks{self._fileUniqSuffix}_results.json"

    @property
    def tasksResultsFileName(self):            
        return f"tasks{self._fileUniqSuffix}_results.json"
        
    @property
    def hypercubeOptions(self):
        return f"--dim {d} --nintervals {m} --nsamplespercell {n}"
    
    def hypercubeGenerate(self):
        args = f"generate {self.hypercubeOptions} --output {self.subtasksFileName}"
        print(f"running: {args}")
        process = subprocess.Popen(self.hypercubeExe + " " + args, shell=True)
        process.wait()
        
    
    def getSubtasks(self):
        # runs hypercube generate to split problem into subtasks
        os.chdir(self.workingDir)
        self.hypercubeGenerate()
        return self.getSubtasksAsLst()
    
    def getSubtasksAsLst(self):
        #read json
        with open(self.subtasksFileName) as f:
            data = json.load(f)
        
        #generate subtasks
        subtasks_ids: list[int] = []
        for it in data:
            subtasks_ids.append(it['taskId']) #TODO should be subtaskId
        return subtasks_ids
    
    def hypercubeRunSubtaskCmd(self, subtask_id):
        cmd = f"{self.hypercubeExe} subtask --id {subtask_id} --input {self.subtasksFileName} --output {self.subtasksResultsFileName}"
        return cmd
        #bash_command = self.changeDirCmd + " && " + run_cmd
        #task = bash.BashOperator(task_id=str(it['taskId']), bash_command = bash_command, pool_slots=1)
        #subtasks.append(task)
        
    def hypercubeGatherCmd(self):
        cmd = f"{self.hypercubeExe} gather --input {self.subtasksResultsFileName} --output {self.tasksResultsFileName}"
        return cmd
            
    @property
    def changeDirCmd(self):
        return f"cd {self.workingDir}"
    
    @property
    def workingDir(self):
        return "/opt/airflow/dags"


from airflow.decorators import task            


with DAG(dag_id="all", schedule="@once", start_date=datetime.datetime.now()) as dag:
    for d in dims:
        for m in mIntervalsPerDim:
            for n in nPointsPerCell:
                h = HypercubeProblem(d, m, n)
                
                @task
                def getSubtasks(hargs):
                    h = HypercubeProblem(*hargs)
                    res = h.getSubtasks()
                    return res
                    
                @task.bash
                def hypercubeRun(hargs, subtask: int):
                    h = HypercubeProblem(*hargs)
                    chdir_cmd = h.changeDirCmd
                    run_cmd = h.hypercubeRunSubtaskCmd(subtask)
                    return chdir_cmd + " && " + run_cmd
                    
                @task.bash
                def reduce(hargs):
                    h = HypercubeProblem(*hargs)
                    chdir_cmd = h.changeDirCmd
                    gather_cmd = h.hypercubeGatherCmd()
                    return chdir_cmd + " && " + gather_cmd
                
                @task
                def generateObj(d, m, n):
                    print(f"d={d}, m={m}, n={n}")
                    #h = HypercubeProblem(d, m, n)
                    #with open(os.path.join(h.workingDir, h.subtasksResultsFileName), "a"):
                    #    pass
                    #with open(os.path.join(h.workingDir, h.tasksResultsFileName), "a"):
                    #    pass
                    return [d, m, n]
                
                args = generateObj(d, m, n)
                subtask_ids = getSubtasks(hargs=args)
                expanded_values = hypercubeRun.partial(hargs=args).expand(subtask=subtask_ids)
                reduce(hargs=args).set_upstream(expanded_values)

if __name__ == "__main__":
    #dag.test()
    print("Ok")