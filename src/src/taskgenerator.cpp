#include <fstream>
#include <TestResultsDB/SubtaskDB.h>

int main () {
    SubtaskDB tasks;
    size_t task_id = 0;
    for(size_t dim = 1; dim <= 3; dim++) {
        for(size_t stride = 1; stride <= dim; stride++) {
            SubtaskParameters subtask{.Ntasks=1,.cur_task=0};
            RandomNumberGeneratorDescription rng_descr{.rng_id=0,.offset=0};
            BitsRepackDescription bits_repack{.src_little_endian=true,.dst_little_endian=true,.bits_per_sample=32};
            size_t mIntervals = 100;
            size_t N = 1;
            for(size_t k = 0; k < dim; k++) {
                N *= mIntervals;
            }
            N *= 100;
            HypercubeProblem problem(dim, mIntervals, stride, N);

            MetaData meta1{.taskId=task_id++};
            tasks.push_back(meta1, rng_descr, std::nullopt, problem, subtask);
            MetaData meta2{.taskId=task_id++};
            tasks.push_back(meta2, rng_descr, bits_repack, problem, subtask);
        }
    }
    tasks.writeToFile("subtasks.json");
    return 0;
}