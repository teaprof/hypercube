#ifndef __TEST_RESULTS_DB_H__
#define __TEST_RESULTS_DB_H__

#include<string>
#include<fstream>
#include<set>
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<serializers/serializers.h>
#include<TestResultsDB/MetaData.h>
#include<TestResultsDB/TaskDB.h>

struct TestResultsRecord : public TaskRecord {
    SubtaskResults results;
};

boost::json::object& operator<<(boost::json::object& object, const TestResultsRecord& record) {
    object<<static_cast<const TaskRecord&>(record);
    object<<record.results;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, TestResultsRecord& record) {
    object>>static_cast<TaskRecord&>(record);
    object>>record.results;
    return object;
}


class TestResultsDB {
    public:

        void readFromFile(const std::string& path) {
            std::ifstream f(path, std::ios::in);
            if(!f) {
                return;
            }
            std::string str;
            f>>str;
            if(str.length() == 0) {
                return;
            }
            auto value = boost::json::parse(str);
            boost::json::value results_value = value.at("results");
            auto array = results_value.as_array();
            for(auto it : array) {                
                TestResultsRecord r;
                it.as_object() >> r;
                records.push_back(std::move(r));
            }
        }
        void writeToFile(const std::string& path) const {
            boost::json::object data;
            boost::json::array array;            
            for(auto it : records) {
                boost::json::object obj;
                obj << it;
                array.push_back(obj);
            }
            data["results"] = array;
            std::ofstream f(path, std::ios::out | std::ios::trunc);
            f<<data;
        }        
        void push_back(const std::optional<MetaData> meta, const RandomNumberGeneratorDescription& rng, const std::optional<BitsRepackDescription> repack, HypercubeProblem& problem, const SubtaskParameters& subtask, const SubtaskResults& results) {
            records.push_back({meta, rng, repack, problem, subtask, results});
        }
        void sanitize() {
            std::set<int> idx_to_remove;
            for(int n = 0; n < records.size(); n++) {
                for(int k = n + 1; k < records.size(); k++) {
                    if(records[n].problem == records[k].problem && records[n].subtask == records[k].subtask) {
                        if(records[n].results == records[k].results) {
                            idx_to_remove.insert(k);
                        } else {
                            std::cout<<"Records number "<<n<<" and "<<k<<" contradics. Removing both of them"<<std::endl;
                            idx_to_remove.insert(n);
                            idx_to_remove.insert(k);
                        }
                    }
                }
            }
            std::vector<int> v(idx_to_remove.begin(), idx_to_remove.end());
            std::sort(v.begin(), v.end(), std::greater());
            for(auto idx : v) {
                records.erase(records.begin() + idx);
            }
        }
        std::vector<TestResultsRecord> getSimilar(const HypercubeProblem& problem, const SubtaskParameters& subtasks) {
            std::vector<TestResultsRecord> results;
            for(auto it : records) {
                if(it.problem == problem && it.subtask.Ntasks == subtasks.Ntasks) {
                    results.push_back(it);
                }
            }
            return results;
        }
        std::vector<TestResultsRecord> getUnique() {
            std::vector<TestResultsRecord> results;
            for(auto it : records) {
                auto pos = std::find_if(results.begin(), results.end(), [&it](auto v1) {
                    return it.problem == v1.problem && it.subtask.Ntasks == v1.subtask.Ntasks;
                });
                if(pos == records.end()) {
                    results.push_back(it);
                }
            }
            return results;
        }
        std::vector<std::vector<TestResultsRecord>> getFinished() {
            std::vector<std::vector<TestResultsRecord>> res;
            auto uniq = getUnique();
            for(auto it : uniq) {
                auto similar = getSimilar(it.problem, it.subtask);
                if(checkComplete(similar)) {
                    res.push_back(similar);
                }
            }
            return res;
        }
        bool checkComplete(const std::vector<TestResultsRecord>& r) {
            if(r.empty()) {
                return false;
            }
            size_t Ntasks = r.begin()->subtask.Ntasks;
            if(r.size() != Ntasks) {
                return false;
            }
            std::set<size_t> subtasks;
            for(auto it : r) {
                subtasks.insert(it.subtask.cur_task);
            }
            for(size_t idx = 0; idx < Ntasks; idx++) {
                if(!subtasks.contains(idx)) {
                    return false;
                }
            }
            return true;
        }
    private:
        std::vector<TestResultsRecord> records;
};

#endif