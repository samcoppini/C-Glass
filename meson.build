project('c-glass', 'c',
    version : '0.1',
    default_options : [
        'c_std=c11',
        'warning_level=3',
        'werror=true',
    ]
)

cc = meson.get_compiler('c')
math_dep = cc.find_library('m', required: false)

if cc.has_argument('-Wshadow')
    add_project_arguments('-Wshadow', language : 'c')
endif

py_mod = import('python3')
python = py_mod.find_python()

subdir('c')
subdir('glass')
