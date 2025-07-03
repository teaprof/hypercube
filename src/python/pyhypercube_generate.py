from hypercube import *
import itertools, math
import tqdm


def createFilesForSubmit(maxMemory):
    maxcells = maxMemory//4
    rng_type = [1]
    dimensions = [1, 2, 3, 4, 5]
    m_intervals_per_dim = [10, 100]
    n_points_per_cell = [10, 20, 50, 100, 1000, 10000, 100000]
    cnt = 0
    for rng, dim, m, n_per_cell in itertools.product(rng_type, dimensions, m_intervals_per_dim, n_points_per_cell):
        rng = RNG(rng, 0)
        total_cells = m ** dim
        total_points = m*total_cells
        problem = HypercubeProblem(dim, m, total_points)
        if total_cells < maxcells:
            nsubtasks = math.ceil(total_cells / maxcells)
            for cursubtask in range(nsubtasks):
                subtask = Subtask(cursubtask, nsubtasks)
                job = HypercubeJob(rng, problem, subtask)
                job.printSubmit(f"{cnt:03}.submit", f"{cnt:03}.pickle")
                cnt += 1
                
if __name__ == '__main__':
    createFilesForSubmit(10*1024**3)
