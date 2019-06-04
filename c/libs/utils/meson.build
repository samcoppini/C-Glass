utils_inc = include_directories('inc')

utils_src = files(
    'src/copy-interface.c',
    'src/list.c',
    'src/map.c',
    'src/set.c',
    'src/stream.c',
    'src/string.c',
)

utils_lib = static_library(
    'utils',
    utils_src,
    include_directories: [utils_inc],
)

utils_dep = declare_dependency(
    include_directories: [utils_inc],
    link_with: [utils_lib],
)

subdir('test')
