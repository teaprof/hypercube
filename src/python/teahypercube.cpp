#include <pybind11/pybind11.h>
#include "../badgenerator/badgenerator.h"
#include "../hypercube/hypercube.h"

namespace py = pybind11;

PYBIND11_MODULE(badgeneratorpy, m)
{
    py::class_<BadGenerator>(m, "BadGenerator")
            .def(py::init<size_t>())
            .def("gen", &BadGenerator::gen, "generate random bit")
            .def("genbyte", [](BadGenerator* self)->uint8_t {return self->genbyte();}, "generate random byte")
            .def("__getitem__", [](BadGenerator* self, size_t index) {return (*self)[index];}, py::is_operator())
            .def("__setitem__", [](BadGenerator* self, size_t index, float v) {(*self)[index] = v;}, py::is_operator());

    py::class_<Hypercube2>(m, "Hypercube")
            .def(py::init<>())
}
