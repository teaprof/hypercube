#include "statearchive.h"
#include <librandom/rng/dynamic/adaptors/adaptor.h>
#include <string>


class RngWithFastForward : public RandomBitAdaptor {
    public:
    RngWithFastForward(std::shared_ptr<RandomBitGenerator> rng, const std::string& filename) : RandomBitAdaptor(rng), filename_(filename) {}

    uint64_t operator()(RandomBitGenerator& rng) override {
        return rng();
    }

    uint16_t nbits() const override {
        return rng()->nbits();
    }

    std::shared_ptr<RandomBitGenerator> copy() const override {
        return std::make_shared<RngWithFastForward>(rng()->copy(), filename_);
    }

    void discardN(uint64_t count) override  {
        //StateArchive arch(filename_, rng()->stateSize());
        std::mt19937 rng;
        for(uint64_t n = 0; n < count; n++)
            RandomBitAdaptor::operator()();
    }
    std::vector<char> state() override {
        throw std::logic_error("Not implemented yet");
    }
    void setState(const std::vector<char>& state) override {
        throw std::logic_error("Not implemented yet");
    }
    size_t stateSize() override {
        throw std::logic_error("Not implemented yet");
    }
    private:
        std::string filename_;
};

