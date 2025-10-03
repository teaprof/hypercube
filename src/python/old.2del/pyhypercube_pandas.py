from hypercube import *
import pandas as pd
import tqdm


def createDF():
    rng_type = pd.Series([0], name = "RNGType")
    dimensions = pd.Series(list(range(1, 5)), name="dim")
    m_intervals_per_dim = pd.Series([10, 100], name="m_intervals")
    n_points_per_cell = pd.Series([10, 20, 50, 100, 1000, 10000, 100000], name='n_points_per_cell')

    df = pd.merge(rng_type, dimensions, 'cross')
    df = pd.merge(df, m_intervals_per_dim, how='cross')
    df = pd.merge(df, n_points_per_cell, how='cross')
    df['TotalCells'] = df['m_intervals'] ** df['dim']
    df['TotalPoints'] = df['TotalCells'] * df['n_points_per_cell']
    df['id'] = df.index
    return df

def removeLong(df):
    df = df[df['TotalPoints'] < 10**7]
    return df

def splitToSubtasks(df, maxMemory=1024**2):
    df2 = pd.DataFrame(columns=('id', 'nsubtasks', 'subtask'))    
    maxCells = maxMemory // 4
    rows = {'id': [], 'nsubtasks': [], 'subtask': []}
    for idx, row in df.iterrows():
        nSubtasks = row['TotalCells'] // maxCells + 1
        for cursubtask in range(nSubtasks):
            rows['id'].append(row['id'])
            rows['nsubtasks'].append(nSubtasks)
            rows['subtask'].append(cursubtask)
    df2 = pd.DataFrame(rows)
    return pd.merge(df, df2)
    

df = createDF()
df = removeLong(df)
df = splitToSubtasks(df)
print(df)

tester = Tester()
for row in tqdm.tqdm(df.itertuples(), total=df.shape[0]):
    #print(row)
    rng = RNG(row.RNGType)
    subtask = Subtask(row.subtask, row.nsubtasks)
    problem = HypercubeProblem(row.dim, row.m_intervals, row.TotalPoints)
    res = tester.runSubtask(rng, problem, subtask)
    

    