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

builtin_funcs_source = custom_target(
    'builtin_funcs_source',
    input: ['data/builtins.txt'],
    output: ['builtin-impl.c'],
    command: [
        python,
        join_paths(meson.current_source_dir(), 'data', 'make_literal.py'),
        '--source', '@INPUT@',
        '--out-name', '@OUTPUT@',
        '--array-name', 'BUILTIN_FUNCS',
    ]
)

main_func_source = custom_target(
    'main_func_source',
    input: ['data/main.txt'],
    output: ['main-impl.c'],
    command: [
        python,
        join_paths(meson.current_source_dir(), 'data', 'make_literal.py'),
        '--source', '@INPUT@',
        '--out-name', '@OUTPUT@',
        '--array-name', 'MAIN_FUNC',
    ]
)

compiler_exe = executable(
    'glasscc',
    compiler_src,
    runtime_source,
    main_func_source,
    builtin_funcs_source,
    dependencies: [glasstypes_dep, parser_dep, utils_dep],
    include_directories: [compiler_inc],
    install: true,
)
