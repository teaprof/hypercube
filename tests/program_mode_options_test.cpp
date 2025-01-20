#include <options/ModeOptions.h>
#include <gtest/gtest.h>

TEST(PROGRAMMODEOPTIONS, PARSE) {
    namespace po = boost::program_options;
    ProgramModesOptions modes;
    PartialOptions runOptions("run group");
    size_t dim;
    runOptions.addPartialVisible("dim,d", po::value<size_t>(&dim)->default_value(2), "hypercube dimension");
    PartialOptions gatherOptions("gather group");
    size_t gather_opt;
    gatherOptions.addPartialVisible("gather,g", po::value<size_t>(&gather_opt)->default_value(2), "some option for gathering");
    PartialOptions commonOptions("common group");
    size_t common_value;
    commonOptions.addPartialVisible("common,c", po::value<size_t>(&common_value)->default_value(2), "common value");
    modes["run"].addGroup(runOptions);
    modes["run"].addGroup(commonOptions);
    modes["gather"].addGroup(gatherOptions);
    modes["gather"].addGroup(commonOptions);

    const char* argv1[] = {"prgmname", "run", "-d", "10",  "-c", "20"};
    modes.parse(6, argv1);
    ASSERT_EQ(dim, 10);
    ASSERT_EQ(common_value, 20);

    const char* argv2[] = {"prgmname", "gather", "-g", "15",  "-c", "30"};
    modes.parse(6, argv2);
    ASSERT_EQ(gather_opt, 15);
    ASSERT_EQ(common_value, 30);

    const char* argv3[] = {"prgmname", "run", "-g", "15",  "-c", "30"};
    EXPECT_THROW ({
            modes.parse(6, argv3);
        },
    po::unknown_option);
}