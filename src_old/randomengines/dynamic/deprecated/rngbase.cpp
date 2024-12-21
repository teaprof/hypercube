#include"rngbase.h"


using namespace mctools;

rngbase::rngbase()
{
    mcount = 0;
}

rngbase::~rngbase()
{
    //nothing to do
}

rngbase::counter_type rngbase::count()
{
    return mcount;
}
