#ifndef __SERIALIZERS_H__
#define __SERIALIZERS_H__
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<stat_tests/hypercube/HypercubeTest.h>
#include<stat_tests/rng.h>
#include<TestResultsDB/MetaData.h>

boost::json::object& operator<<(boost::json::object& object, const RandomNumberGeneratorDescription& rng_descr) {
    object.emplace("rng_id", rng_descr.rng_id);
    if(rng_descr.seed) {
        object.emplace("seed", *rng_descr.seed);
    } else {
        object["seed"].emplace_null();
    }
    object.emplace("offset", rng_descr.offset);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, RandomNumberGeneratorDescription& rng_descr) {
    rng_descr.rng_id = object.at("rng_id").as_int64();
    auto v = object.at("seed");
    if(v.is_null()) {
        rng_descr.seed = std::nullopt;
    } else {
        assert(v.is_int64());
        rng_descr.seed = v.as_int64();
    }
    rng_descr.offset = object.at("offset").as_int64();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const BitsRepackDescription& bits_repack_descr) {
    object.emplace("repack_bits_per_sample", bits_repack_descr.bits_per_sample);
    object.emplace("repack_src_little_endian", bits_repack_descr.src_little_endian);
    object.emplace("repack_dst_little_endian", bits_repack_descr.dst_little_endian);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, BitsRepackDescription& bits_repack_descr) {
    bits_repack_descr.bits_per_sample = object.at("repack_bits_per_sample").as_int64();
    bits_repack_descr.src_little_endian = object.at("repack_src_little_endian").as_bool();
    bits_repack_descr.dst_little_endian =object.at("repack_dst_little_endian").as_bool();
    return object;
}


boost::json::object& operator<<(boost::json::object& object, const Chi2BasedProblem& problem) {
    object.emplace("N", problem.N);
    object.emplace("m_intervals_total", problem.m_intervals_total);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, Chi2BasedProblem& problem) {
    problem.N = object.at("N").as_int64();
    problem.m_intervals_total = object.at("m_intervals_total").as_int64();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const StatiscticalTestResults& results) {
    object.emplace("dof", results.dof);
    object.emplace("mean", results.mean);
    object.emplace("sum", results.sum);
    object.emplace("sum2", results.sum2);
    object.emplace("N", results.N);
    object.emplace("chi2", results.chi2);
    object.emplace("chi2cdf", results.chi2cdf);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, StatiscticalTestResults& problem) {
    problem.dof = object.at("dof").as_int64();
    problem.mean = object.at("mean").as_double();
    problem.sum = object.at("sum").as_int64();
    problem.sum2 = object.at("sum2").as_int64();
    problem.N = object.at("N").as_int64();
    problem.chi2 = object.at("chi2").as_double();
    problem.chi2cdf = object.at("chi2cdf").as_double();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const SubtaskParameters& subtask) {
    object.emplace("Ntasks", subtask.Ntasks);
    object.emplace("cur_task", subtask.cur_task);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskParameters& subtask) {
    subtask.Ntasks = object.at("Ntasks").as_int64();
    subtask.cur_task = object.at("cur_task").as_int64();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const SubtaskResults& results) {
    object.emplace("sum", results.sum);
    object.emplace("sum2", results.sum2);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskResults& problem) {
    problem.sum = object.at("sum").as_int64();
    problem.sum2 = object.at("sum2").as_int64();
    return object;
}


boost::json::object& operator<<(boost::json::object& object, const HypercubeProblem& problem) {
    object.emplace("dim", problem.dim);
    object.emplace("m_intervals_per_dim", problem.m_intervals_per_dim);
    object.emplace("stride", problem.stride);
    object << static_cast<const Chi2BasedProblem&>(problem);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, HypercubeProblem& problem) {
    problem.dim = object.at("dim").as_int64();
    problem.m_intervals_per_dim = object.at("m_intervals_per_dim").as_int64();
    problem.stride = object.at("stride").as_int64();
    object >> static_cast<Chi2BasedProblem&>(problem);
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const MetaData& meta) {
    object.emplace("taskID", meta.taskId);
    if(meta.parentId) {
        object.emplace("parentId", *meta.parentId);
    } else {
        object["parentId"].emplace_null();
    }
    boost::json::array childIds;
    for(auto it : meta.childIds) {
        childIds.emplace_back(it);
    }
    object.emplace("childIds", childIds);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, MetaData& meta) {
    meta.taskId = object.at("taskID").as_int64();
    auto v = object.at("parentId");
    if(v.is_null()) {
        meta.parentId = std::nullopt;
    } else {
        assert(v.is_int64());
        meta.parentId = v.as_int64();
    }
    boost::json::array childIds = object.at("childIds").as_array();
    meta.childIds.clear();
    for(auto it : childIds) {
        meta.childIds.push_back(it.as_int64());
    }
    return object;
}

template<class T>
boost::json::object& operator<<(boost::json::object& object, const std::optional<T> value_opt) {
    if(value_opt) {
        object<<*value_opt;
    }
    return object;
}

template<class T>
const boost::json::object& operator>>(const boost::json::object& object, std::optional<T>& value_opt) {
    try {
        T value;
        object>>value;
        value_opt = value;
    } catch (const boost::system::system_error&) {
        value_opt = std::nullopt;
    }    
    return object;
}


#endif