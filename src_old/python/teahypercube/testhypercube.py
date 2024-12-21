#import badgeneratorpy as BG
#g = BG.BadGenerator(4)
#b = g.genbyte()
#print(type(b), bin(b))

import numpy as np
import teahypercube as t
h = t.Hypercube(10, 2)
#h.setIndexGenerator(True, 1, 0);
r = np.random.randint(0, 255, size=1000, dtype='uint8')
h.run(r)
