import os, subprocess, json 

class HypercubeProblem:
    tasks_dir= "/opt/airflow/dags/files"
    subtasks_dir= "/opt/airflow/dags/files/subtasks"
    tasks_results_file_name = os.path.join(tasks_dir, "tasks_results.json")
    problems_file_name = os.path.join(tasks_dir, "problems.json")

    def __init__(self, dim, nIntervalsPerDim, nPointsPerCell):
        self.dim = dim
        self.nIntervalsPerDim = nIntervalsPerDim
        self.nPointsPerCell = nPointsPerCell
        
    def fileds(self):
        return {"dim": self.dim, "nIntervalsPerDim": self.nIntervalsPerDim, "nPointsPerCell": self.nPointsPerCell}
    
    @staticmethod
    def fromFields(fields):
        dim = fields["dim"]
        nIntervalsPerDim = fields["nIntervalsPerDim"]
        nPointsPerCell = fields["nPointsPerCell"]
        return HypercubeProblem(dim, nIntervalsPerDim, nPointsPerCell)
        
    @property
    def hypercubeExe(self):
        #return "./bin/hypercube"
        return "hypercube"
        
    @property
    def _fileUniqSuffix(self):
        return f"d{self.dim}m{self.nIntervalsPerDim}nps{self.nPointsPerCell}"
    
    @property
    def subtasksFileName(self):            
        return os.path.join(self.subtasks_dir, f"subtasks{self._fileUniqSuffix}.json")

    @property
    def subtasksResultsFileName(self):            
        return os.path.join(self.subtasks_dir, f"subtasks{self._fileUniqSuffix}_results.json")
            
    @property
    def hypercubeOptions(self):
        return f"--dim {self.dim} --nintervals {self.nIntervalsPerDim} --nsamplespercell {self.nPointsPerCell}"
    
    def hypercubeGenerate(self):
        args = f"generate {self.hypercubeOptions} --output {self.subtasksFileName}"
        print(f"running: {args}")
        process = subprocess.Popen(self.hypercubeExe + " " + args, shell=True)
        process.wait()
    
    def getSubtasks(self):
        # runs hypercube generate to split problem into subtasks
        os.chdir(self.tasks_dir)
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
        
    def hypercubeGatherCmd(self):
        cmd = f"{self.hypercubeExe} gather --input {self.subtasksResultsFileName} --output {self.tasks_results_file_name}"
        return cmd
            
    
