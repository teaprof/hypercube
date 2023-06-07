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

class GeneralOptions
{
public:
    GeneralOptions() : visible("General options")
    {
        cmdline.add_options()
                ("help,h", po::bool_switch(&showHelp), "produce help message");
        visible.add(cmdline);
    }
    void validate()
    {

    }
    po::options_description visible;
    po::options_description cmdline;
    bool showHelp;
};

class HypercubeOptions
{
public:
    HypercubeOptions() : visible("Hypercube-test options")
    {
        cmdline.add_options()
                ("dim,d", po::value<int>(&dim)->default_value(3), "dimension, positive integer: 1, 2, ...")
                ("intervals,m", po::value<int>(&mIntervals)->default_value(16), "split number of each dimension: 2, 3, ...")
                ("npoints,N", po::value<int>(&NPoints)->default_value(100000), "number of points, positive integer");
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

class StrategyOptions
{
public:
    StrategyOptions() : visible("Strategy options")
    {
        cmdline.add_options()
                ("stride,s", po::value<int>(&stride)->default_value(0), "stride, specify 0 to make stride equal to dimension")
                ("binary,b", po::bool_switch(&binarymode), "use RNG in binary mode (mIntervals should be power of two)")
                ("float,f", po::value(&bytesPerFloat)->implicit_value(4), "convert each N random bytes to floating point value in [0, 1)");
        visible.add(cmdline);
    }
    void validate(const po::variables_map& vm)
    {
        if(stride < 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
        if(bytesPerFloat && bytesPerFloat < 0)
            throw po::validation_error(po::validation_error::invalid_option_value);
        if(binarymode && bytesPerFloat)
            throw po::validation_error(po::validation_error::invalid_option_value, "-f and -b should not be specified simultaneously");
        if(!binarymode)
            bytesPerFloat = 4;
    }
    int stride;
    bool binarymode;
    boost::optional<int> bytesPerFloat;
    po::options_description cmdline;
    po::options_description visible;
};

class RandomNumberGeneratorOptions
{
public:
    RandomNumberGeneratorOptions() : visible("Reandom-number-generator options")
    {
        visible.add_options()
                ("generator,g", po::value(&gentypeopt), "generator type: 0, 1, 2. If 0, then filename should be specified. Default is 1.")
                ("seed,e", po::value(&seed), "seed, int64_t")
                ("jump,j", po::value(&jump), "jump, unsigned integer")
                ("list,l", po::bool_switch(&list), "list all random number generators and exit");
        cmdline.add(visible);
        cmdline.add_options()
                ("input-file", po::value(&filename), "input file");
        positional.add("input-file", 1);
    }
    void validate(const po::variables_map& vm)
    {
        if(gentypeopt && gentypeopt.value() < 0)
            throw po::validation_error(po::validation_error::invalid_option_value, "generator should be zero or positive integer value");
        if(filename)
        {
            if(gentypeopt && gentypeopt.value() != 0)
                throw po::validation_error(po::validation_error::invalid_option_value, "generator type should be 0 or unspecified if filename is used");
            gentype = 0;
        } else {
            if(gentypeopt)
                gentype = gentypeopt.value();
            else
                gentype = 1;
        }
    }
    boost::optional<int> gentypeopt;
    int gentype;
    boost::optional<int64_t> seed;
    size_t jump;
    bool list;
    po::options_description cmdline;
    po::options_description visible;
    po::positional_options_description positional;
    boost::optional<std::string> filename;
};


class OutputOptions
{
public:
    OutputOptions() : visible("Ouput options")
    {
        cmdline.add_options()
                ("verbose,v", po::bool_switch(&verbose), "verbose");
        visible.add(cmdline);
    }
    void validate()
    {

    }
    po::options_description visible;
    po::options_description cmdline;
    bool verbose;
};


class Options
{
public:
    GeneralOptions go;
    HypercubeOptions ho;
    StrategyOptions so;
    RandomNumberGeneratorOptions ro;
    OutputOptions oo;

    Options(int argc, char* argv[])
    {
        initialize(argc, argv);
    }

    void initialize(int argc, char* argv[])
    {
        po::options_description cmdline_options;
        cmdline_options.add(go.cmdline).add(ho.cmdline).add(ro.cmdline).add(so.cmdline).add(oo.cmdline);

        po::variables_map vm;
        po::command_line_parser parser(argc, argv);
        parser.options(cmdline_options).positional(ro.positional);
        po::store(parser.run(), vm);
        po::notify(vm);

        ho.validate(vm);
        so.validate(vm);
        ro.validate(vm);

    }
    int help()
    {
        cout << "Usage: " << endl;
        cout << "hypercube [options] [source]" << endl;
        cout << go.visible << endl;
        cout << ho.visible << endl;
        cout << so.visible << endl;
        cout << ro.visible << endl;
        cout << oo.visible << endl;
        cout << "source\t name of the file to be used of random source."<<endl;
        return 1;
    }

};


std::shared_ptr<RandomBitGenerator> createRandomBitGenerator(Options& opt)
{
    switch(opt.ro.gentype)
    {
        case 0:
            assert(opt.ro.filename);
            return std::make_shared<FileBitGenerator>(opt.ro.filename.value(), opt.oo.verbose);
        break;
        case 1:
            static std::mt19937 mt;
            return std::make_shared<StdRandomBitGenerator<std::mt19937>>(mt, opt.oo.verbose);
        break;
        default:
            throw po::validation_error(po::validation_error::invalid_option_value, "generator type should be 0 or 1");
    }
}

std::shared_ptr<IndexGeneratorBase<RandomBitGenerator>> createIndexGenerator(Options& opt)
{
    if(opt.so.binarymode)
    {
        if(opt.so.stride == 0)
        {
            opt.so.stride = opt.ho.dim*std::log2(opt.ho.mIntervals);
            if(opt.oo.verbose)
                std::cout<<"Setting stride to "<<opt.so.stride<<" bits"<<std::endl;
        }
        return std::make_shared<BinaryIndexGenerator>(opt.ho.dim, opt.ho.mIntervals, opt.so.stride);
    } else {
        return std::make_shared<FloatingPointIndexGenerator>(opt.ho.dim, opt.ho.mIntervals, opt.so.stride, opt.so.bytesPerFloat.value());
    }
}

double runHypercube(Options& opt)
{
    if(opt.oo.verbose)
    {
        std::cout<<"Generator: "<<opt.ro.gentype<<std::endl;
        std::cout<<"Dimension: "<<opt.ho.dim<<std::endl;
        std::cout<<"mIntervals: "<<opt.ho.mIntervals<<std::endl;
        if(opt.so.stride == 0)
            std::cout<<"Stride: "<<opt.so.stride<<" (see below)"<<std::endl;
        else
            std::cout<<"Stride: "<<opt.so.stride<<std::endl;
        std::cout<<"NPoints: "<<opt.ho.NPoints<<std::endl;
    }
    std::shared_ptr<RandomBitGenerator> rng = createRandomBitGenerator(opt);
    std::shared_ptr<IndexGeneratorBase<RandomBitGenerator>> gen = createIndexGenerator(opt);
    HypercubeTest2 ht(opt.ho.dim, opt.ho.NPoints, opt.ho.mIntervals, *gen);
    return ht(*rng, opt.oo.verbose);
}


int main(int argc, char* argv[])
{
    Options opt(argc, argv);
    if (opt.go.showHelp)
        return opt.help();

    double res = runHypercube(opt);
    std::cout<<"res = "<<res<<std::endl;
    return 0;
}
