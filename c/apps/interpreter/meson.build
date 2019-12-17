interpreter_inc = include_directories('inc')

interpreter_src = files(
    'src/glass-instance.c',
    'src/glass-value.c',
    'src/interpreter.c',
    'src/main.c',
)

interpreter_exe = executable(
    'cglass',
    interpreter_src,
    dependencies: [glasstypes_dep, math_dep, parser_dep, utils_dep],
    include_directories: [interpreter_inc],
    install: true,
)
