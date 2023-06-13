import badgeneratorpy as BG
g = BG.BadGenerator(4)
b = g.genbyte()
print(type(b), bin(b))