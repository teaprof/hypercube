#ifndef __TEST_RESULTS_DB_H__
#define __TEST_RESULTS_DB_H__

#include<string>
#include<fstream>
#include<set>
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<serializers/serializers.h>

class TestResultsDB {
    public:
        struct Record {
            HypercubeProblem problem;
            SubtaskParameters subtask;
            SubtaskResults results;
        };

        TestResultsDB() = default;
        void readFromFile(std::string path) {
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
                auto r = fromJSONobject(it.as_object());
                records.push_back(std::move(r));
            }
        }
        void writeToFile(std::string path) {
            boost::json::object data;
            boost::json::array array;            
            for(auto it : records) {
                auto obj = toJSONObject(it);
                array.push_back(obj);
            }
            data["results"] = array;
            std::ofstream f(path, std::ios::out | std::ios::trunc);
            f<<data;
        }        
        void push_back(const HypercubeProblem& problem, const SubtaskParameters& subtask, const SubtaskResults& results) {
            records.push_back({problem, subtask, results});
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
        std::vector<Record> getSimilar(const HypercubeProblem& problem, const SubtaskParameters& subtasks) {
            std::vector<Record> results;
            for(auto it : records) {
                if(it.problem == problem && it.subtask.Ntasks == subtasks.Ntasks) {
                    results.push_back(it);
                }
            }
            return results;
        }
        std::vector<Record> getUnique() {
            std::vector<Record> results;
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
        std::vector<std::vector<Record>> getFinished() {
            std::vector<std::vector<Record>> res;
            auto uniq = getUnique();
            for(auto it : uniq) {
                auto similar = getSimilar(it.problem, it.subtask);
                if(checkComplete(similar)) {
                    res.push_back(similar);
                }
            }
            return res;
        }
        bool checkComplete(const std::vector<Record>& r) {
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
        std::vector<Record> records;

        boost::json::object toJSONObject(const Record& r) {
            boost::json::object obj;
            obj<<r.problem<<r.subtask<<r.results;
            return obj;
        }
        Record fromJSONobject(const boost::json::object& obj) {            
            HypercubeProblem problem{0, 0, 0, 0};
            SubtaskParameters subtask{0, 0, 0};
            SubtaskResults results{0, 0};
            obj>>problem>>subtask>>results;
            return {problem, subtask, results};
        }
};

#endif