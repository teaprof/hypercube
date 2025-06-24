/* File : pyhypercube.i */
%module pyhypercube
%{
#include "Hypercube.h"
%}

%include "stdint.i"
%include "stl.i"

namespace std {
    %template(DVector) vector<double>;
    %template(FVector) vector<float>;
    %template(UInt8Vector) vector<uint8_t>;
}

%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& restored_codeword};
%apply int& OUTPUT {int& actualnumiters};
%apply std::vector<uint8_t> &OUTPUT {std::vector<uint8_t>& finalparitychecks};
%apply std::vector<uint8_t> &INPUT {const std::vector<uint8_t>& msg};

%include "Hypercube.h"
