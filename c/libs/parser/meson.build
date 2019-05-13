parser_inc = include_directories('inc')

parser_src = files(
    'src/parser.c',
)

parser_lib = static_library(
    'parser',
    parser_src,
    include_directories: parser_inc,
    dependencies: [glasstypes_dep, utils_dep],
)

parser_dep = declare_dependency(
    include_directories: [parser_inc],
    link_with: [parser_lib],
)

subdir('test')
