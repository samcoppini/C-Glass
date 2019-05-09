test_inc = include_directories('inc')

test_src = files(
    'src/test.c'
)

test_lib = static_library(
    'tests',
    test_src,
    include_directories: test_inc,
)

test_dep = declare_dependency(
    include_directories: [test_inc],
    link_with: [test_lib],
)
