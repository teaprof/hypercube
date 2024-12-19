#include <fstream>
#include <optional>
#include <vector>

template <class RNG> class Rewinder {
public:
  struct Header {
    size_t nStates;
  };
  struct StateHeader {
    size_t offset;
  };

  Rewinder(std::string filename) : filename_(std::move(filename)){};
  ~Rewinder(){};

  void readHeaders() {
    std::ifstream f(filename_, std::ios::in | std::ios::binary);
    f.seekg(sizeof(Header), std::ios_base::end);
    f.read(reinterpret_cast<char *>(&header), sizeof(header));
    state_headers.assign(header.nStates, StateHeader{0});
    f.seekg(header.nStates * sizeof(StateHeader) + sizeof(header),
            std::ios_base::end);
    f.read(reinterpret_cast<char *>(state_headers.data()),
           sizeof(StateHeader) * header.nStates);
  }

  RNG rewind(size_t offset) {
    std::optional<size_t> best_idx;
    for (size_t i = 0; i < state_headers.size(); i++) {
      if (state_headers[i].offset < offset) {
        if (!best_idx.has_value() ||
            state_headers[i].offset > *state_headers[best_idx]) {
          best_idx = i;
        }
      }
    }
    std::ifstream f(filename_, std::ios::in | std::ios::binary);
    // f.seekg(best_idx*sizeof());
  }
  Header header;
  std::vector<StateHeader> state_headers;
  std::string filename_;
};

int main() {}