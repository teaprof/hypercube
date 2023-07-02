import argparse, mmap, tqdm

import numpy as np

from hypercube import Hypercube

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("filename")
    parser.add_argument("-k", "--dim", default=4, type=int)
    parser.add_argument("--order", default="big", choices=["big", "little"])

    args = parser.parse_args()

    print(f"Filename: {args.filename}")
    print(f"Dim: {args.dim}")
    print(f"order: {args.order}")

    with open(args.filename, "r+b") as f:
        mm = mmap.mmap(f.fileno(), 0)

    assert len(mm) >= 1024, "file is too small, at least 1025 bytes are required"

    bits = np.unpackbits(mm, bitorder=args.order)
    dim = args.dim
    H = Hypercube(dim)
    H.fill(bits)

    print(f"Entropy: {H.H2()}")

