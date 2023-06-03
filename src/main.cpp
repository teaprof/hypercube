#include <iostream>
#include <random>
#include <bitset>
#include <boost/program_options.hpp>
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
                ("dim,d", po::value<int>(&dim)->default_value(3), "dimension, positive integer: 1, 2, ...")
                ("interval,m", po::value<int>(&mIntervals)->default_value(10), "split number of each dimension: 2, 3, ...")
                ("nsamples,N", po::value<int>(&NPoints)->default_value(100000), "number of points, positive integer");
        visible.add(cmdline);
    }
    void validate()
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
    RandomNumberGeneratorOptions() : visible("Reandon-number-generator options")
    {
        visible.add_options()
                ("stride,s", po::value<int>(&s)->default_value(0), "stride, specify 0 to make stride equal to dimension");
        cmdline.add(visible);
        cmdline.add_options()
                ("input-file", po::value<std::string>(), "input file");
        positional.add("input-file", 1);
    }
    void validate()
    {
        if(s < 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
    }
    int s; //we use signed int because boost::program_options::value don't make difference between signed and unsigned integers.
    po::options_description cmdline;
    po::options_description visible;
    po::positional_options_description positional;
};

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

    hcubeOptions.validate();
    rngOptions.validate();

    if (vm.count("help")) {
        cout << "Usage: " << endl;
        cout << "hypercube [options] [source]" << endl;
        cout << generalOptions << endl;
        cout << hcubeOptions.visible << endl;
        cout << rngOptions.visible << endl;
        cout << "source\t name of the file to be used of random source."<<endl;
        return 1;
    }

    if(hcubeOptions.mIntervals == 2)
    {
        std::mt19937 mt;
        using RandomBitGenerator = CaterpillarBitGenerator<std::mt19937>;
        RandomBitGenerator randbitgen(mt);
        BinaryHypercubeTest<RandomBitGenerator> ht(hcubeOptions.dim, hcubeOptions.NPoints);
        std::cout<<"res = "<<ht(randbitgen)<<std::endl;
    } else {
        std::mt19937 mt;
        using RandomBitGenerator = stdRandomNumberEngine<decltype(mt)>;
        RandomBitGenerator randbitgen(mt);
        HypercubeTest<RandomBitGenerator> ht(hcubeOptions.dim, hcubeOptions.NPoints, hcubeOptions.mIntervals);
        std::cout<<"res = "<<ht(randbitgen)<<std::endl;
    }
    return 0;
}
