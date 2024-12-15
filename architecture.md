# Hypercube parameters

- dim
- nIntervals
- seed
- N points
- multiindex strategy (k-independent, overlapping)
- total subtasks
- subtask number

# Strategies

## Histogtam data container

* simple no-multithread
* mutex protected blocks
* atomic protected block 


## BitToFloat

## BitToInt

## BitToBool

for a special case when nIntervals = 2

## MultiindexGenerator

MultiindexGenerator.operator(rng) -> size_t

* KIndependentGenerator
* ChainGenerator


## RandomNumberGeneratorWrapper


## Chi2Test

parameters:
* container
* multiindex generator
* random number generator

