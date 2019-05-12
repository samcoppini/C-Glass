glasstypes_inc = include_directories('inc')

glasstypes_src = files(
    'src/glass-class.c',
    'src/glass-command.c',
    'src/glass-function.c',
)

glasstypes_lib = static_library(
    'glasstypes',
    glasstypes_src,
    include_directories: [glasstypes_inc],
    dependencies: [utils_dep],
)

glasstypes_dep = declare_dependency(
    include_directories: [glasstypes_inc],
    link_with: [glasstypes_lib],
)
