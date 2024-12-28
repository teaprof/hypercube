#ifndef __TEST_RESULTS_DB_H__
#define __TEST_RESULTS_DB_H__

#include<string>
#include<fstream>
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>

class TestResultsDB {
    public:
        TestResultsDB() = default;
        void readFromFile(std::string path) {
            
        }
        void writeToFile(std::string path) {
            boost::json::object data;
            boost::json::array array;            
            for(auto it : records) {
                boost::json::object obj;
                auto problem = std::get<0>(it);
                auto subtask = std::get<1>(it);
                auto results = std::get<2>(it);
                obj["m_intervals_total"] = problem.m_intervals_total;
                obj["N"] = problem.N;
                array.push_back(obj);
            }
            std::ofstream f(path, std::ios::out | std::ios::binary);
            f<<array;
        }        
        void push_back(const Chi2BasedProblem& problem, const SubtaskParameters& subtask, const SubtaskResults& results) {
            records.push_back({problem, subtask, results});
        }
    private:
        std::vector<std::tuple<Chi2BasedProblem, SubtaskParameters, SubtaskResults>> records;
};

#endif