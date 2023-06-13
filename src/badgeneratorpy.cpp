#include <pybind11/pybind11.h>
#include "badgenerator.h"

namespace py = pybind11;

PYBIND11_MODULE(hypercube, m)
{
    py::class_<BadGenerator>(m, "BadGenerator")
            .def(py::init<size_t>())
            .def("__getitem__", [](const float value, size_t index) {return m[index];}, py::is_operator());
}
