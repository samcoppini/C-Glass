project('c-glass', 'c',
    version : '0.1',
    default_options : ['warning_level=3']
)

cc = meson.get_compiler('c')

if cc.has_argument('-Wshadow')
    add_project_arguments('-Wshadow', language : 'c')
endif

py_mod = import('python3')
python = py_mod.find_python()

subdir('c')
subdir('glass')
