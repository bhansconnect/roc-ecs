project(
    'ecs-test',
    'cpp',
    version : '0.1',
    default_options : [
        'warning_level=2',
        'cpp_std=c++17',
        'default_library=static',
    ]
)

sdl2_dep = dependency('sdl2')

executable('ecs-test', 'main.cc', dependencies: sdl2_dep)

