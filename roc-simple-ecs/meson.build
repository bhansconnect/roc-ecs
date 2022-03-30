project(
    'roc-simple-ecs-test',
    'cpp',
    version : '0.1',
    default_options : [
        'warning_level=2',
        'cpp_std=c++17',
        'default_library=static',
    ]
)

project_dir  = meson.current_source_dir()
cc = meson.get_compiler('cpp')

lib_roc_simple_ecs = cc.find_library('roc-simple-ecs', dirs : project_dir)

sdl2_dep = dependency('sdl2')

executable(
    'roc-simple-ecs',
    'main.cc',
    dependencies: [
        sdl2_dep,
        lib_roc_simple_ecs,
    ],
)

