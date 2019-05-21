project('c-glass', 'c',
    version : '0.1',
    default_options : ['warning_level=3']
)

cc = meson.get_compiler('c')

if cc.has_argument('-Wshadow')
    add_project_arguments('-Wshadow', language : 'c')
endif

subdir('c')
