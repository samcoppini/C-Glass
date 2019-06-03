minifier_inc = include_directories('inc')

minifier_src = files(
    'src/main.c',
    'src/minification.c',
)

minifier_exe = executable(
    'glassmin',
    minifier_src,
    dependencies: [glasstypes_dep, parser_dep, utils_dep],
    include_directories: [minifier_inc],
    install: true,
)
