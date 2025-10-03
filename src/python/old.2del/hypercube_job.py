import pyhypercube
import argparse, pickle

print("123")

if __name__ == '__main__':
    print("1230")
    exit()
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    args = parser.parse_args()
    
    with open(argparse.filename, "rb") as f:
        job = pickle.load(f)
    
    res = job.run()
    print("Ok")
    print(f"{res.sum} {res.sum2}")