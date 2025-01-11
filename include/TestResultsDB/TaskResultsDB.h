#ifndef __TASK_RESULTS_DB__
#define __TASK_RESULTS_DB__

#include "TestResultsDB/SubtasksResultsDB.h"
#include "stat_tests/chi2based/StatisticalTestBase.h"

struct TaskResultsRecord : public TaskRecord {
    StatiscticalTestResults results;
    bool operator==(const TaskResultsRecord& other) const {
        return static_cast<const TaskRecord&>(*this) == other && results == other.results;
    }
};

boost::json::object& operator<<(boost::json::object& object, const TaskResultsRecord& record) {
    object<<static_cast<const TaskRecord&>(record)<<record.results;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, TaskResultsRecord& record) {
    object>>static_cast<TaskRecord&>(record)>>record.results;    
    return object;
}


class TaskResultsDB {
    public:
        void readFromFile(const std::string& path) {
            std::ifstream f(path, std::ios::in);
            if(!f) {
                throw std::runtime_error("Can't open file for reading");
            }
            auto value = boost::json::parse(f);
            boost::json::value results_value = value.at("tasks");
            auto array = results_value.as_array();
            for(auto it : array) {                
                TaskResultsRecord r;
                try {
                    it.as_object() >> r;
                    records_.push_back(std::move(r));
                } catch (const boost::system::system_error&) {
                    std::cout<<"one record has incorrect format and will be ignored"<<std::endl;
                }
            }
        }
        void push_back(TaskResultsRecord& task_results) {
            records_.push_back(std::move(task_results));
        }
        void sanitize() {
            std::set<int> idx_to_remove;
            for(size_t n = 0; n < records_.size(); n++) {
                for(size_t k = n + 1; k < records_.size(); k++) {
                    TaskRecord& r1 = static_cast<TaskRecord&>(records_[n]);
                    TaskRecord& r2 = static_cast<TaskRecord&>(records_[k]);
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
        void writeToFile(const std::string& path) const {
            boost::json::object data;
            boost::json::array array;            
            for(auto it : records_) {
                boost::json::object obj;
                obj << it;
                array.push_back(obj);
            }
            data["tasks"] = array;
            std::ofstream f(path, std::ios::out | std::ios::trunc);
            if(!f) {
                throw std::runtime_error("can't open file for writing");
            }
            f<<data;
        }        
        std::optional<TaskResultsRecord> find(uint64_t taskId) const {
            for(auto it : records_) {
                if(it.meta && it.meta->taskId == taskId) 
                    return it;
            }
            return std::nullopt;
        }
        const std::vector<TaskResultsRecord>& records() {
            return records_;
        }
    private:
    std::vector<TaskResultsRecord> records_;
};

#endif