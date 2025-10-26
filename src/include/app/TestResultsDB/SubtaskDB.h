#ifndef __TASK_DB_H__
#define __TASK_DB_H__

#include<app/TestResultsDB/MetaData.h>
#include<app/serializers/jsonserializers.h>

#include<stattests/stat_tests/chi2based/StatisticalTestBase.h>
#include<stattests/stat_tests/rng.h>

#include<boost/json.hpp>

#include<string>
#include<fstream>
#include<set>
#include<optional>

struct TaskRecord {
    std::optional<MetaData> meta;
    RandomNumberGeneratorDescription rng;
    std::optional<BitsRepackDescription> repack;
    HypercubeProblem problem;
    std::optional<RNGArchiveDescription> rng_archive_description;

    bool operator==(const TaskRecord& other) const {
        return meta == other.meta && rng == other.rng && repack == other.repack && problem == other.problem;
    }
};

struct SubtaskRecord : public TaskRecord {
    SubtaskParameters subtask;
    bool operator==(const SubtaskRecord& other) const {
        return static_cast<const TaskRecord&>(*this) == other && subtask == other.subtask;
    }
};


boost::json::object& operator<<(boost::json::object& object, const TaskRecord& record) {
    object<<record.meta<<record.rng<<record.repack<<record.problem;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, TaskRecord& record) {
    object>>record.meta>>record.rng>>record.repack>>record.problem;
    return object;
}


boost::json::object& operator<<(boost::json::object& object, const SubtaskRecord& record) {
    object<<static_cast<const TaskRecord&>(record);
    object<<record.subtask;
    return object;
}

const boost::json::object& operator>>(const boost::json::object& object, SubtaskRecord& record) {
    object>>static_cast<TaskRecord&>(record);
    object>>record.subtask;
    return object;
}


class SubtaskDB {
    public:
        void readFromFile(const std::string& path) {
            std::ifstream f(path, std::ios::in);
            if(!f) {
                throw std::runtime_error("Can't open file for reading");
            }
            auto value = boost::json::parse(f);
            //boost::json::value results_value = value.at("subtasks");
            //auto array = results_value.as_array();
            auto array = value.as_array();
            for(auto it : array) {                
                SubtaskRecord r;
                it.as_object() >> r;
                records_.push_back(std::move(r));
            }
        }
        void push_back(const std::optional<MetaData> meta, const RandomNumberGeneratorDescription& rng, std::optional<BitsRepackDescription> bits_repack, const HypercubeProblem& problem, std::optional<RNGArchiveDescription> rng_archive, const SubtaskParameters& subtask) {
            SubtaskRecord v{meta, rng, bits_repack, problem, rng_archive, subtask};
            records_.push_back(std::move(v));
        }
        void writeToFile(const std::string& path) const {
            boost::json::array array;            
            for(auto it : records_) {
                boost::json::object obj;
                obj << it;
                array.push_back(obj);
            }
            std::ofstream f(path, std::ios::out | std::ios::trunc);
            if(!f) {
                throw std::runtime_error("can't open file for writing");
            }
            f<<array;
        }        
        std::optional<SubtaskRecord> find(uint64_t taskId) const {
            for(auto it : records_) {
                if(it.meta && it.meta->taskId == taskId) 
                    return it;
            }
            return std::nullopt;
        }
        const std::vector<SubtaskRecord>& records() {
            return records_;
        }
    private:
        std::vector<SubtaskRecord> records_;
};

#endif