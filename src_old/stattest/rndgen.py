import time

import hypercube
import argparse
import numpy as np
import tqdm

if __name__ == '__main__':
    parser = argparse.ArgumentParser(prog="rndgen", description="generator of (bad) random numbers")
    parser.add_argument('filename', help="name of file for writing random bytes")
    parser.add_argument('-n', '--count', default=10000, help="number of bytes", type=int)
    parser.add_argument('-d', '--dim', default=20, help="dimensionality", type=int)
    parser.add_argument('-m', '--modify', nargs='*', help="modification of hypercube values, format: \"regex,delta\"")

    args = parser.parse_args()
    #args = parser.parse_args(["bad2.bin", "-m",".*0,1"])
    H = hypercube.Hypercube(args.dim)

    #parsing --modify option
    patterns = []; deltas = []
    if args.modify is not None:
        print(args.modify)
        for modstr in args.modify:
            strlist = modstr.split(",")
            if len(strlist) != 2:
                raise ValueError(f"incorrect value for --modify: {modstr}, it should contain two comma-separated fields")
            patterns.append(strlist[0])
            deltas.append(float(strlist[1]))

    idx = H.sampleInitialIndex()

    for patternstr, delta in zip(patterns, deltas):
        H.modifyProbabilities(patternstr, delta)


    res = np.zeros(args.count*8, dtype=np.uint8)
    t1 = time.perf_counter()
    for n in tqdm.tqdm(range(args.count*8)):
        idx, bit = H.sampleNextBit(idx)
        res[n] = bit
    t2 = time.perf_counter()
    print(f"Generating finished in {t2-t1} secs")

    with open(args.filename, "wb") as f:
        f.write(np.packbits(res))





