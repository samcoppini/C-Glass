compiler_inc = include_directories('inc')

compiler_src = files(
    'src/compiler.c',
    'src/main.c',
)

compiler_exe = executable(
    'glasscc',
    compiler_src,
    dependencies: [glasstypes_dep, parser_dep, utils_dep],
    include_directories: [compiler_inc],
    install: true,
)
