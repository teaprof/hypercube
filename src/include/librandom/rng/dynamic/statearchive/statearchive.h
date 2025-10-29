#include<boost/interprocess/sync/file_lock.hpp>
#include<algorithm>
#include<fstream>
#include<string>
#include<cstdint>
#include<vector>
#include<optional>
#include<cassert>
#include<mutex>
#include<filesystem>

class StateArchive {
    public:
        struct HeaderElement {
            uint64_t rng_offset;  // offset of the rng from the initial state
            uint64_t position; // position from the begging of file
        };

        StateArchive(const std::string path, size_t state_size) : path_(path), state_size_(state_size) 
        {
            m.lock();
            f.open(path_, std::ios::out | std::ios::app); //create file if it is not exists            
            f.close();
            f.open(path_, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
            flock = boost::interprocess::file_lock(path.c_str());
            header_elements_ = std::move(loadHeader());
            // check file consistency            
            for(auto h : header_elements_) {
                loadState(h);
            }
        }
        ~StateArchive() {
            f.flush(); 
            flock.unlock();
            f.close();
            m.unlock();
        }
        
        void addState(const std::vector<char>& state, uint64_t rng_offset, size_t minimal_distance = 0) {
            if(minimal_distance > 0) {
                std::optional<HeaderElement> prev_state = getProxyState(rng_offset);
                if(prev_state.has_value() && (rng_offset - prev_state.value().rng_offset < minimal_distance))
                    return;
            }
            auto it = std::find_if(header_elements_.begin(), header_elements_.end(), [&](auto& it) {return it.rng_offset == rng_offset;});
            if(it != header_elements_.end()) {
                return;
            }
            if(std::filesystem::file_size(path_) == 0) {
                f.clear();
                if(writeState(state)) {
                    uint64_t pos = 0;
                    header_elements_.push_back(HeaderElement{rng_offset, pos});
                    if(writeHeader(header_elements_)) {
                        return;
                    }                    
                }
            } else {
                assert(f.good());
                if(seekHeaderStartP()) {
                    uint64_t pos = f.tellp();
                    if(writeState(state)) {
                        header_elements_.push_back(HeaderElement{rng_offset, pos});
                        if(writeHeader(header_elements_)) {
                            return;
                        }                    
                    }
                }
            }
            std::stringstream str;
            str<<"Can't seekp, remove file "<<path_;
            throw std::runtime_error(str.str());
        }

        std::vector<char> loadState(const HeaderElement& element) {
            assert(f.is_open());
            f.seekg(element.position);
            std::vector<char> res(state_size_);
            f.read(reinterpret_cast<char*>(res.data()), state_size_);
            uint64_t check_sum;
            f.read(reinterpret_cast<char*>(&check_sum), sizeof(check_sum));
            if(check_sum != checksum(res)) 
                throw std::runtime_error("checksum is incorrect");
            return res;
        }

        std::optional<HeaderElement> getProxyState(uint64_t required_rng_offset) {
            //return element with maximum rng_offset not exceeding required_state
            std::optional<HeaderElement> best_state;
            for(const auto& it : header_elements_) {
                if(it.rng_offset <= required_rng_offset)
                    if(!best_state.has_value() || it.rng_offset >= best_state.value().rng_offset) {
                        best_state = it;
                    }
            }
            return best_state;
        }

    private:
        std::fstream f;
        boost::interprocess::file_lock flock;
        std::mutex m; // for thread synchronization which is not provided by boost::interprocess::file_lock

        std::streamoff headerStartPos(size_t n_elements) {
            //return the starting position of the header assuming that it consists of n_elements elements
            //the position is relative to the end of file since header is placed at the end of the file
            if(n_elements > std::numeric_limits<uint32_t>::max()) 
                throw std::runtime_error("too many header elements");
            uint32_t n_elements32 = static_cast<uint32_t>(n_elements);
            int64_t pos = sizeof(n_elements32) + n_elements32*sizeof(HeaderElement);
            return -pos; //from the end of file
        }

        bool seekHeaderStartP() {
            assert(f.is_open());
            auto pos = headerStartPos(header_elements_.size());
            f.seekp(pos, std::ios::end);
            return f.good();
        }
        bool seekHeaderStartG(size_t n_elements) {
            assert(f.is_open());
            auto pos = headerStartPos(n_elements);
            f.seekg(pos, std::ios::end);
            return f.good();
        }
        bool writeState(const std::vector<char>& state) {             
            assert(state.size() == state_size_);
            f.write(reinterpret_cast<const char*>(state.data()), state_size_);
            uint64_t check_sum = checksum(state);
            f.write(reinterpret_cast<char*>(&check_sum), sizeof(check_sum));
            return f.good();
        }
        std::vector<HeaderElement> loadHeader() {
            assert(f.is_open());
            uint32_t n_elements;
            f.seekg(-sizeof(n_elements), std::ios::end);
            f.read(reinterpret_cast<char*>(&n_elements), sizeof(n_elements));
            seekHeaderStartG(n_elements);
            std::vector<HeaderElement> elements(n_elements);
            f.read(reinterpret_cast<char*>(elements.data()), elements.size()*sizeof(HeaderElement));
            if(!f.good()) {
                // leave file in good state                
                f.clear();
                return {};
            }
            return elements;
        }
        bool writeHeader(const std::vector<HeaderElement>& elements_) {
            uint32_t n_elements = elements_.size();
            f.write(reinterpret_cast<const char*>(header_elements_.data()), n_elements*sizeof(HeaderElement));
            f.write(reinterpret_cast<const char*>(&n_elements), sizeof(n_elements));            
            return f.good();
        }

        uint64_t checksum(const std::vector<char>& data) {
            uint64_t res = 0;
            for(auto it : data)
                res+=it;
            return res;
        }

        std::vector<HeaderElement> header_elements_;
        std::string path_;
        size_t state_size_;
};