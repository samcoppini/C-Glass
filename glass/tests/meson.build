tests = [
    ['builtin-A-test', 'builtin-math.glass', 'builtin-math.out'],
    ['builtin-S-test', 'builtin-str.glass',  'builtin-str.out' ],
    ['list-test',      'list-test.glass',    'list-test.out'   ],
    ['map-test',       'map-test.glass',     'map-test.out'    ],
    ['string-test',    'string-test.glass',  'string-test.out' ],
]

foreach test : tests
    test_name = test[0]
    test_src  = test[1]
    test_out  = test[2]

    test(
        test_name,
        python,
        args: [
            './test-interpreter.py',
            '--exe', interpreter_exe.full_path(),
            '--expected', test_out,
            test_src,
        ] + glass_lib_paths,
        workdir: meson.current_source_dir(),
        suite: ['glass-test'],
    )

endforeach
