/*
    tests/test_keep_alive.cpp -- keep_alive modifier (pybind11's version
    of Boost.Python's with_custodian_and_ward / with_custodian_and_ward_postcall)

    Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#include "pybind11_tests.h"

class Child {
public:
    Child() { std::cout << "Allocating child." << std::endl; }
    ~Child() { std::cout << "Releasing child." << std::endl; }
};

class Parent {
public:
    Parent() { std::cout << "Allocating parent." << std::endl; }
    ~Parent() { std::cout << "Releasing parent." << std::endl; }
    void addChild(Child *) { }
    Child *returnChild() { return new Child(); }
    Child *returnNullChild() { return nullptr; }
};

void init_ex_keep_alive(py::module &m) {
    py::class_<Parent>(m, "Parent")
        .def(py::init<>())
        .def("addChild", &Parent::addChild)
        .def("addChildKeepAlive", &Parent::addChild, py::keep_alive<1, 2>())
        .def("returnChild", &Parent::returnChild)
        .def("returnChildKeepAlive", &Parent::returnChild, py::keep_alive<1, 0>())
        .def("returnNullChildKeepAliveChild", &Parent::returnNullChild, py::keep_alive<1, 0>())
        .def("returnNullChildKeepAliveParent", &Parent::returnNullChild, py::keep_alive<0, 1>());

    py::class_<Child>(m, "Child")
        .def(py::init<>());
}
