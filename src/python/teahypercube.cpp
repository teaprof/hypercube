#include <pybind11/pybind11.h>
#include <pybind11/buffer_info.h>
#include "../badgenerator/badgenerator.h"
#include "../hypercube/hypercube.h"

namespace py = pybind11;

PYBIND11_MODULE(teahypercube, m)
{
    py::class_<BadGenerator>(m, "BadGenerator", py::buffer_protocol())
            .def(py::init<size_t>())
            .def("gen", &BadGenerator::gen, "generate random bit")
            .def("genbyte", [](BadGenerator* self)->uint8_t {return self->genbyte();}, "generate random byte")
            .def("__getitem__", [](BadGenerator* self, size_t index) {return (*self)[index];}, py::is_operator())
            .def("__setitem__", [](BadGenerator* self, size_t index, float v) {(*self)[index] = v;}, py::is_operator())
            .def("size", [](BadGenerator* self) {return self->size();})
            .def_buffer([](BadGenerator& g) -> py::buffer_info {
                return py::buffer_info(
                            g.data(),
                            sizeof(float),
                            py::format_descriptor<float>::format(),
                            1,
                            {g.size()},
                            {sizeof(float)}
                            );
    });

    py::class_<HypercubeTestPy>(m, "Hypercube", py::buffer_protocol())
            .def(py::init<size_t, size_t>())
            .def("setIndexGenerator", [](HypercubeTestPy* self, bool bin, size_t stride, size_t bytesPerFloat = 0)
                {
                    self->setIndexGenerator(bin, stride, bytesPerFloat);
                })
            .def("run", [](HypercubeTestPy* self, py::buffer v)->double
                {
                    py::buffer_info info = v.request();
                    if(info.format != py::format_descriptor<uint8_t>::format())
                        throw std::runtime_error("Incompatible format: expected array of uint8");
                    if(info.ndim != 1)
                        throw std::runtime_error("Incompatible buffer dimension");
                    size_t size = info.size/info.itemsize;
                    return self->run(static_cast<const char*>(info.ptr), size);
                })
            .def_buffer([](HypercubeTestPy& self) -> py::buffer_info
                {
                    return py::buffer_info(
                                self.data(),
                                sizeof(size_t),
                                py::format_descriptor<size_t>::format(),
                                1,
                                {self.size()},
                                {sizeof(size_t)}
                                );
                });
}
