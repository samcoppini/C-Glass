compiler_inc = include_directories('inc')

compiler_src = files(
    'src/compiler.c',
    'src/main.c',
)

runtime_source = custom_target(
    'runtime_source',
    input: ['data/runtime.txt'],
    output: ['runtime-literal.c'],
    command: [
        python,
        join_paths(meson.current_source_dir(), 'data', 'make_literal.py'),
        '--source', '@INPUT@',
        '--out-name', '@OUTPUT@',
        '--array-name', 'RUNTIME_LIBRARY',
    ]
)

compiler_exe = executable(
    'glasscc',
    compiler_src,
    runtime_source,
    dependencies: [glasstypes_dep, parser_dep, utils_dep],
    include_directories: [compiler_inc],
    install: true,
)
