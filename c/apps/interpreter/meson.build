interpreter_inc = include_directories('inc')

interpreter_src = files(
    'src/main.c',
)

interpreter = executable(
    'cglass',
    interpreter_src,
    dependencies: [utils_dep],
    install: true,
)
