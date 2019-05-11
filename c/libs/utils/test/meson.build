test_files = [
    ['list',   'list-test.c'  ],
    ['map',    'map-test.c'   ],
    ['string', 'string-test.c'],
]

foreach test: test_files

test_name = test[0]
test_src = test[1]

test_exe = executable(
    test_name + '-test',
    test_src,
    dependencies: [utils_dep, test_dep],
)

test(test_name + '-test', test_exe)

endforeach
