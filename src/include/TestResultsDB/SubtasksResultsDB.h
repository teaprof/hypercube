#ifndef __SUBTASKS_RESULTS_DB_H__
#define __SUBTASKS_RESULTS_DB_H__

#include<string>
#include<fstream>
#include<set>
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<serializers/jsonserializers.h>
#include<TestResultsDB/MetaData.h>
#include<TestResultsDB/SubtaskDB.h>

struct SubtaskResultsRecord : public SubtaskRecord {
    SubtaskResults results;
    double time;
};

boost::json::object& operator<<(boost::json::object& object, const SubtaskResultsRecord& record) {
    object<<static_cast<const SubtaskRecord&>(record);
    object<<record.results;
    object["time"] = record.time;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskResultsRecord& record) {
    object>>static_cast<SubtaskRecord&>(record);
    object>>record.results;
    record.time = object.at("time").as_double();
    return object;
}


class SubtaskResultsDB {
    public:
        void readFromFile(const std::string& path) {
            std::ifstream f(path, std::ios::in);
            if(!f) {
                return;
            }
            auto value = boost::json::parse(f);
            boost::json::value results_value = value.at("results");
            auto array = results_value.as_array();
            for(auto it : array) {                
                SubtaskResultsRecord r;
                it.as_object() >> r;
                records_.push_back(std::move(r));
            }
        }
        void writeToFile(const std::string& path) const {
            boost::json::object data;
            boost::json::array array;            
            for(auto it : records_) {
                boost::json::object obj;
                obj << it;
                array.push_back(obj);
            }
            data["results"] = array;
            std::ofstream f(path, std::ios::out | std::ios::trunc);
            f<<data;
        }        
        void push_back(const SubtaskRecord& task, const SubtaskResults& results, double time) {
            records_.push_back({task.meta, task.rng, task.repack, task.problem, task.subtask, results, time});
        }
        void add(const std::vector<SubtaskRecord> &tasks, const std::vector<SubtaskResults>& results, const std::vector<double> times) {
            assert(tasks.size() == results.size());
            assert(tasks.size() == times.size());
            for(size_t n = 0; n < tasks.size(); n++) {
                auto& task = tasks[n];
                auto& res = results[n];
                auto& time = times[n];
                push_back(task, res, time);
            }
            sanitize();
        }
        void sanitize() {
            std::set<int> idx_to_remove;
            for(int n = 0; n < records_.size(); n++) {
                for(int k = n + 1; k < records_.size(); k++) {
                    auto r1 = static_cast<SubtaskRecord&>(records_[n]);
                    auto r2 = static_cast<SubtaskRecord&>(records_[k]);
                    if(r1 == r2) {
                        if(records_[n].results == records_[k].results) {
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
                records_.erase(records_.begin() + idx);
            }
        }
        std::vector<SubtaskResultsRecord> getSimilar(const SubtaskRecord& subtask) const {
            std::vector<SubtaskResultsRecord> results;
            for(auto it : records_) {
                if(subtask == it) {
                    results.push_back(it);
                }
            }
            return results;
        }
        std::vector<SubtaskResultsRecord> getUnique() const {
            std::vector<SubtaskResultsRecord> results;
            for(auto it : records_) {
                auto pos = std::find_if(results.begin(), results.end(), [&it](auto v1) {
                    return static_cast<SubtaskRecord>(it) == v1 && it.subtask.Ntasks == v1.subtask.Ntasks;
                });
                if(pos == results.end()) {
                    results.push_back(it);
                }
            }
            return results;
        }
        std::vector<std::vector<SubtaskResultsRecord>> getFinished() const {
            std::vector<std::vector<SubtaskResultsRecord>> res;
            auto uniq = getUnique();
            for(auto it : uniq) {
                auto similar = getSimilar(it);
                if(checkComplete(similar)) {
                    res.push_back(similar);
                }
            }
            return res;
        }
        bool checkComplete(const std::vector<SubtaskResultsRecord>& r) const {
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
        std::vector<SubtaskResultsRecord> records_;
};

#endif