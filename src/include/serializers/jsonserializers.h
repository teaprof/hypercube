#ifndef __SERIALIZERS_H__
#define __SERIALIZERS_H__
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<stat_tests/hypercube/HypercubeTest.h>
#include<stat_tests/rng.h>
#include<TestResultsDB/MetaData.h>

// This is a sentinel mechanism that should detect if new fields have been added to structure `S`.
// The implementation is based on the comparison of the structure size and total size of all fields that
// are expected to be the only fields of the structure. Of course, this can't detect all possible modifications
// of the original structure, but in many cases this mechanism is sufficient to detect modifications that are
// important for correct serialization or hash calculation.
template<typename S, typename ... Args>
struct AssertOnlyFields {
    static constexpr size_t size_expected = (... + sizeof(Args));
    static_assert(size_expected==sizeof(S), "The structure have been changed, please update calling function");
    AssertOnlyFields(const S& s, const Args& ... args) {}
};

template<typename BaseClass, typename S, typename ... Args>
struct AssertFieldsAndBaseClass {
    static constexpr size_t size_expected = (... + sizeof(Args)) + sizeof(BaseClass);
    static_assert(size_expected==sizeof(S), "The structure have been changed, please update calling function");
    AssertFieldsAndBaseClass(const S& s, size_t BaseClassSize, const Args& ... args) {
        
    }
};



boost::json::object& operator<<(boost::json::object& object, const RandomNumberGeneratorDescription& rng_descr) {
    AssertOnlyFields(rng_descr, rng_descr.offset, rng_descr.rng_id, rng_descr.seed);
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
    AssertOnlyFields(rng_descr, rng_descr.offset, rng_descr.rng_id, rng_descr.seed);
    rng_descr.rng_id = object.at("rng_id").to_number<uint64_t>();
    auto v = object.at("seed");
    if(v.is_null()) {
        rng_descr.seed = std::nullopt;
    } else {
        assert(v.is_int64());
        rng_descr.seed = v.to_number<uint64_t>();
    }
    rng_descr.offset = object.at("offset").to_number<uint64_t>();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const BitsRepackDescription& bits_repack_descr) {
    AssertOnlyFields(bits_repack_descr, bits_repack_descr.bits_per_sample, bits_repack_descr.src_little_endian, bits_repack_descr.dst_little_endian);
    if(bits_repack_descr.bits_per_sample) {
        object.emplace("repack_bits_per_sample", *bits_repack_descr.bits_per_sample);        
    } else {
        object["repack_bits_per_sample"].emplace_null();
    }
    object.emplace("repack_src_little_endian", bits_repack_descr.src_little_endian);
    object.emplace("repack_dst_little_endian", bits_repack_descr.dst_little_endian);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, BitsRepackDescription& bits_repack_descr) {
    AssertOnlyFields(bits_repack_descr, bits_repack_descr.bits_per_sample, bits_repack_descr.src_little_endian, bits_repack_descr.dst_little_endian);
    auto v = object.at("repack_bits_per_sample");
    if(v.is_null()) {
        bits_repack_descr.bits_per_sample = std::nullopt;
    } else {
        assert(v.is_int64());
        bits_repack_descr.bits_per_sample = v.to_number<uint64_t>();
    }    
    bits_repack_descr.src_little_endian = object.at("repack_src_little_endian").as_bool();
    bits_repack_descr.dst_little_endian =object.at("repack_dst_little_endian").as_bool();
    return object;
}


boost::json::object& operator<<(boost::json::object& object, const Chi2BasedProblem& problem) {
    AssertOnlyFields(problem, problem.N, problem.n_cells_total);
    object.emplace("N", problem.N);
    object.emplace("n_cells_total", problem.n_cells_total);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, Chi2BasedProblem& problem) {
    AssertOnlyFields(problem, problem.N, problem.n_cells_total);
    problem.N = object.at("N").to_number<uint64_t>();
    problem.n_cells_total = object.at("n_cells_total").to_number<uint64_t>();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const StatiscticalTestResults& results) {
    AssertOnlyFields(results, results.dof, results.mean, results.sum, results.sum2, results.N, results.chi2, results.chi2cdf);
    object.emplace("dof", results.dof);
    object.emplace("mean", results.mean);
    object.emplace("sum", results.sum);
    object.emplace("sum2", results.sum2);
    object.emplace("N", results.N);
    object.emplace("chi2", results.chi2);
    object.emplace("chi2cdf", results.chi2cdf);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, StatiscticalTestResults& results) {
    AssertOnlyFields(results, results.dof, results.mean, results.sum, results.sum2, results.N, results.chi2, results.chi2cdf);
    results.dof = object.at("dof").to_number<uint64_t>();
    results.mean = object.at("mean").as_double();
    results.sum = object.at("sum").to_number<uint64_t>();
    results.sum2 = object.at("sum2").to_number<uint64_t>();
    results.N = object.at("N").to_number<uint64_t>();
    results.chi2 = object.at("chi2").as_double();
    results.chi2cdf = object.at("chi2cdf").as_double();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const SubtaskParameters& subtask) {
    AssertOnlyFields(subtask, subtask.Ntasks, subtask.cur_task);
    object.emplace("Ntasks", subtask.Ntasks);
    object.emplace("cur_task", subtask.cur_task);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskParameters& subtask) {
    AssertOnlyFields(subtask, subtask.Ntasks, subtask.cur_task);
    subtask.Ntasks = object.at("Ntasks").to_number<uint64_t>();
    subtask.cur_task = object.at("cur_task").to_number<uint64_t>();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const SubtaskResults& results) {
    AssertOnlyFields(results, results.sum, results.sum2);
    object.emplace("sum", results.sum);
    object.emplace("sum2", results.sum2);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskResults& results) {
    AssertOnlyFields(results, results.sum, results.sum2);
    results.sum = object.at("sum").to_number<uint64_t>();
    results.sum2 = object.at("sum2").to_number<uint64_t>();
    return object;
}


boost::json::object& operator<<(boost::json::object& object, const HypercubeProblem& problem) {
    AssertOnlyFields(problem, static_cast<const Chi2BasedProblem&>(problem), problem.dim, problem.m_intervals_per_dim, problem.stride);
    object.emplace("dim", problem.dim);
    object.emplace("m_intervals_per_dim", problem.m_intervals_per_dim);
    object.emplace("stride", problem.stride);
    object << static_cast<const Chi2BasedProblem&>(problem);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, HypercubeProblem& problem) {
    AssertOnlyFields(problem, static_cast<const Chi2BasedProblem&>(problem), problem.dim, problem.m_intervals_per_dim, problem.stride);
    problem.dim = object.at("dim").to_number<uint64_t>();
    problem.m_intervals_per_dim = object.at("m_intervals_per_dim").to_number<uint64_t>();
    problem.stride = object.at("stride").to_number<uint64_t>();
    object >> static_cast<Chi2BasedProblem&>(problem);
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const MetaData& meta) {    
    AssertOnlyFields(meta, meta.taskId);
    object.emplace("taskId", meta.taskId);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, MetaData& meta) {
    AssertOnlyFields(meta, meta.taskId);
    meta.taskId = object.at("taskId").to_number<uint64_t>();
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
    } catch (const boost::system::system_error& ec) {
        value_opt = std::nullopt;
    }    
    return object;
}


#endif