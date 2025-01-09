#ifndef __TASK_DB_H__
#define __TASK_DB_H__

#include<string>
#include<fstream>
#include<set>
#include<optional>
#include<boost/json.hpp>
#include<stat_tests/chi2based/StatisticalTestBase.h>
#include<stat_tests/rng.h>
#include<serializers/jsonserializers.h>

struct TaskRecord {
    std::optional<MetaData> meta;
    RandomNumberGeneratorDescription rng;
    std::optional<BitsRepackDescription> repack;
    HypercubeProblem problem;
    SubtaskParameters subtask;
};

boost::json::object& operator<<(boost::json::object& object, const TaskRecord& record) {
    object<<record.meta<<record.rng<<record.repack<<record.problem<<record.subtask;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, TaskRecord& record) {
    object>>record.meta>>record.rng>>record.repack>>record.problem>>record.subtask;
    return object;
}


class TaskDB {
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
                TaskRecord r;
                it.as_object() >> r;
                records_.push_back(std::move(r));
            }
        }
        void push_back(const std::optional<MetaData> meta, const RandomNumberGeneratorDescription& rng, std::optional<BitsRepackDescription> bits_repack, const HypercubeProblem& problem, const SubtaskParameters& subtask) {
            records_.push_back({meta, rng, bits_repack, problem, subtask});
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
        std::optional<TaskRecord> find(uint64_t taskId) const {
            for(auto it : records_) {
                if(it.meta && it.meta->taskId == taskId) 
                    return it;
            }
            return std::nullopt;
        }
        const std::vector<TaskRecord>& records() {
            return records_;
        }
    private:
        std::vector<TaskRecord> records_;
};

#endif