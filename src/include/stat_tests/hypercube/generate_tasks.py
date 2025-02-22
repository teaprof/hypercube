import json

dims = range(1, 21)
nSlices = 10
maxMemory = 120*1024*1024*1024 #bytes
bytes_per_cell = 2;
totalCells = maxMemory*nSlices/bytes_per_cell
average_hits_per_cell = 1000;
N_points = totalCells*average_hits_per_cell

tasks = []
task_id = 0
for dim in dims:
    for stride in range(1, dim+1):
        m_intervals_per_dim = int(totalCells ** (1/dim))        
        task = {}
        task["id"] = task_id
        task["dim"] = dim
        task["stride"] = stride
        task["m_intervals_per_dim"] = m_intervals_per_dim
        task["N_points"] = N_points
        task["N_tasks"] = nSlices
        tasks.append(task)
        task_id += 1
        
str = json.dumps(tasks, indent=2)
print(str)