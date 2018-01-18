cmake_minimum_required (VERSION 3.9 FATAL_ERROR)

project(BibTexFormat)

add_executable(
    BibTexFormat MACOSX_BUNDLE
    main.cpp
)

set_property(TARGET BibTexFormat PROPERTY CXX_STANDARD 17)
set_property(TARGET BibTexFormat PROPERTY CXX_STANDARD_REQUIRED On)
