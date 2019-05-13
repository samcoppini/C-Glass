parser_test_src = files(
    'parser-test.c',
)

parser_test_exe = executable(
    'parser-test',
    parser_test_src,
    dependencies: [glasstypes_dep, parser_dep, test_dep, utils_dep],
)

test('parser-test', parser_test_exe)
