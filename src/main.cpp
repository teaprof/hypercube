#include <iostream>
#include <random>
#include <bitset>
#include <memory>
#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include "hypercubetest.h"
#include "randombitgenerator.h"

using namespace std;

namespace po = boost::program_options;

class HypercubeOptions
{
public:
    HypercubeOptions() : visible("Hypercube-test options")
    {
        cmdline.add_options()
                ("dim,d", po::value<int>(&dim)->implicit_value(3), "dimension, positive integer: 1, 2, ...")
                ("interval,m", po::value<int>(&mIntervals)->default_value(10), "split number of each dimension: 2, 3, ...")
                ("nsamples,N", po::value<int>(&NPoints)->implicit_value(100000), "number of points, positive integer");
        visible.add(cmdline);
    }
    void validate(const po::variables_map& vm)
    {
        if(dim <= 0 || mIntervals <= 1 || NPoints <= 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
    }
    int dim, mIntervals, NPoints; //we use signed int because boost::program_options::value don't make difference between signed and unsigned integers.
    po::options_description cmdline;
    po::options_description visible;
};

class RandomNumberGeneratorOptions
{
public:
    RandomNumberGeneratorOptions() : visible("Reandom-number-generator options")
    {
        visible.add_options()
                ("stride,s", po::value<int>(&stride)->implicit_value(0), "stride, specify 0 to make stride equal to dimension")
                ("generator,g", po::value<int>(&gentype)->implicit_value(1), "stride, specify 0 to make stride equal to dimension")
                ("binary,b", po::bool_switch(&binarymode), "use RNG in binary mode (mIntervals should be power of two)")
                ("float,f", po::value(&bytesPerFloat)->implicit_value(4), "convert each N random bytes to floating point value in [0, 1)");
        cmdline.add(visible);
        cmdline.add_options()
                ("input-file", po::value<std::string>(), "input file");
        positional.add("input-file", 1);
    }
    void validate(const po::variables_map& vm)
    {
        if(stride < 0 || gentype < 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
        if(bytesPerFloat && bytesPerFloat < 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
        if(binarymode && bytesPerFloat)
            throw po::validation_error(po::validation_error::invalid_option_value, "-f and -b should not be specified simultaneously");
    }
    int gentype;
    int stride;
    bool binarymode;
    boost::optional<int> bytesPerFloat;
    po::options_description cmdline;
    po::options_description visible;
    po::positional_options_description positional;
};

std::shared_ptr<RandomBitGenerator> createRandomBitGenerator(const HypercubeOptions& ho, const RandomNumberGeneratorOptions& ro)
{
    static std::mt19937 mt;
    return std::make_shared<StdRandomBitGenerator>(mt);
}

std::shared_ptr<IndexGeneratorBase> createIndexGenerator(const HypercubeOptions& ho, const RandomNumberGeneratorOptions& ro)
{
    if(ro.binarymode)
    {
        return std::make_shared<BinaryIndexGenerator>(ho.dim, ho.mIntervals, ro.stride);
    } else {
        return std::make_shared<FloatingPointIndexGenerator>(ho.dim, ho.mIntervals, ro.stride, ro.bytesPerFloat);
    }
}

double runBinaryHypercube(const HypercubeOptions& ho, const RandomNumberGeneratorOptions& ro)
{
    std::shared_ptr<RandomBitGenerator> rng = createRandomBitGenerator(ho, ro);
    std::shared_ptr<IndexGeneratorBase> gen = createIndexGenerator(ho, ro);
    HypercubeTest2<RandomBitGenerator> ht(hcubeOptions.dim, hcubeOptions.NPoints);
    return ht(rng);
}

double runFloatingPointHypercube(const HypercubeOptions& ho, const RandomNumberGeneratorOptions& ro)
{
    IndexGenerator& gen = createIndexGenerator(ho, ro);
    RandomBitGenerator randbitgen(gen);
    HypercubeTest<RandomBitGenerator> ht(hcubeOptions.dim, hcubeOptions.NPoints);
    return ht(randbitgen);
}

int main(int argc, char* argv[])
{
    po::options_description generalOptions("General options");
    generalOptions.add_options()
            ("help,h", "produce help message");

    HypercubeOptions hcubeOptions;
    RandomNumberGeneratorOptions rngOptions;

    po::options_description cmdline_options;
    cmdline_options.add(generalOptions).add(hcubeOptions.cmdline).add(rngOptions.cmdline);

    po::variables_map vm;
    po::command_line_parser parser(argc, argv);
    parser.options(cmdline_options).positional(rngOptions.positional);
    po::store(parser.run(), vm);
    po::notify(vm);

    hcubeOptions.validate(vm);
    rngOptions.validate(vm);

    if (vm.count("help")) {
        cout << "Usage: " << endl;
        cout << "hypercube [options] [source]" << endl;
        cout << generalOptions << endl;
        cout << hcubeOptions.visible << endl;
        cout << rngOptions.visible << endl;
        cout << "source\t name of the file to be used of random source."<<endl;
        return 1;
    }

    if(rngOptions.b)
    {
        runBinaryHypercube(hcubeOptions, rngOptions);
    } else {
        std::mt19937 mt;
        using RandomBitGenerator = stdRandomNumberEngine<decltype(mt)>;
        RandomBitGenerator randbitgen(mt);
        HypercubeTest<RandomBitGenerator> ht(hcubeOptions.dim, hcubeOptions.NPoints, hcubeOptions.mIntervals);
        std::cout<<"res = "<<ht(randbitgen)<<std::endl;
    }
    return 0;
}
