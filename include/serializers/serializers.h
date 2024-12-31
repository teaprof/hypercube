#ifndef __SERIALIZERS_H__
#define __SERIALIZERS_H__
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<stat_tests/hypercube/HypercubeTest.h>

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
    object.emplace("chi2", results.chi2);
    object.emplace("sum", results.sum);
    object.emplace("sum2", results.sum2);
    object.emplace("N", results.N);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, StatiscticalTestResults& problem) {
    problem.dof = object.at("dof").as_int64();
    problem.chi2 = object.at("chi2").as_double();
    problem.sum = object.at("sum").as_int64();
    problem.sum2 = object.at("sum2").as_int64();
    problem.N = object.at("N").as_int64();
    return object;
}

boost::json::object& operator<<(boost::json::object& object, const SubtaskParameters& subtask) {
    object.emplace("Ntasks", subtask.Ntasks);
    object.emplace("cur_task", subtask.cur_task);
    object.emplace("n_threads", subtask.n_threads);
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskParameters& subtask) {
    subtask.Ntasks = object.at("Ntasks").as_int64();
    subtask.cur_task = object.at("cur_task").as_int64();
    subtask.n_threads = object.at("n_threads").as_int64();
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


#endif