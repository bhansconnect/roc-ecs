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

pcg_proj = subproject('pcg')
pcg_dep = pcg_proj.get_variable('pcg_cpp_dep')

executable(
    'simple-ecs-test',
    'main.cc',
    dependencies: [
        sdl2_dep,
        pcg_dep,
    ],
    cpp_args: [
        '-DSIMPLE_ECS'
    ]
)

executable(
    'acton-ecs-test',
    'main.cc',
    dependencies: [
        sdl2_dep,
        pcg_dep,
    ],
    cpp_args: [
        '-DACTON_ECS'
    ]
)

executable(
    'roc-ecs-test',
    'main.cc',
    dependencies: [
        sdl2_dep,
        pcg_dep,
    ],
    cpp_args: [
        '-DROC_ECS'
    ]
)

