#ifndef __PRETTY_SERIALIZERS_H__
#define __PRETTY_SERIALIZERS_H__

#include "TestResultsDB/SubtaskDB.h"
#include <ostream>


std::ostream& operator<<(std::ostream& str, const SubtaskRecord& task) {
    str<<"rng id = "<<task.rng.rng_id<<std::endl;
    if(task.rng.seed) {
        str<<"rng seed = "<<*task.rng.seed<<std::endl;
    } else {
        str<<"rng seed = default "<<std::endl;
    }
    str<<"rng offset = "<<task.rng.offset<<std::endl;
    if(task.repack) {
        str<<"repack sample size = "<<task.repack->bits_per_sample<<std::endl;
        str<<"repack srcLittleEndian = "<<task.repack->src_little_endian<<std::endl;
        str<<"repack dstLittleEndian = "<<task.repack->dst_little_endian<<std::endl;
    } else {
        str<<"no bits repack is used"<<std::endl;
    }
    str<<"dim = "<<task.problem.dim<<std::endl;
    str<<"nIntervals = "<<task.problem.m_intervals_per_dim<<std::endl;
    str<<"nIntervalsTotal = "<<task.problem.m_intervals_total<<std::endl;
    str<<"nSamples = "<<task.problem.N<<std::endl;

    str<<"nSubtask = "<<task.subtask.Ntasks<<std::endl;
    str<<"current subtask = "<<task.subtask.cur_task<<std::endl;
    return str;
}

#endif