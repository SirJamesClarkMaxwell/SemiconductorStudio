project('JFM', 'cpp', 'cuda',
    version: '1.0.0')

cpp = meson.get_compiler('cpp')
cu = meson.get_compiler('cuda')

subdir('JFMApp')
subdir('JFMServices')
subdir('JunctionFitMaster')
subdir('Vendor')

subs = [
    ['JFMApp', JFMApp_sources, JFMApp_includes],
    ['JFMServices', JFMServices_sources, JFMServices_includes],
    ['JunctionFitMaster', JunctionFitMaster_sources, JunctionFitMaster_includes],
    ['Vendor', [], Vendor_includes]
]

JFM_sources = []
JFM_includes = ['.']

foreach sub : subs
    name = sub[0]

    foreach _source : sub[1]
        JFM_sources += name + '/' + _source
    endforeach

    foreach _include : sub[2]
        JFM_includes += name + '/' + _include
    endforeach
endforeach

project_root = meson.source_root()

linked_deps = [
    '-L' + join_paths(project_root, 'Vendor/imgui/build'), '-limgui',
    '-L' + join_paths(project_root, 'Vendor/implot/build'), '-limplot',
    '-L' + join_paths(project_root, 'Vendor/LambertW/build'), '-lLambertW',
    '-L' + join_paths(project_root, 'Vendor/yaml-cpp/build'), '-lyaml-cpp',
]
deps = []

args = ['-Wall',
        '-Wextra',
        '-Wformat',
        '-Werror',
        '-Wno-unused-variable',
        '-Wno-unused-function',
        '-Wno-unused-parameter',
        '-Wno-format-security',
        '-Wno-tautological-overlap-compare',
        '-Wno-unused-but-set-variable',
        '-fexperimental-library']

use_debug = false
if get_option('debug')
    use_debug = true
    args += '-DJFM_DEBUG'
endif

mode = get_option('mode')
if mode.contains('single-core')
    args += '-DJFM_MODE_SINGLE_CORE'
elif mode.contains('multi-core')
    args += '-DJFM_MODE_MULTI_CORE'
elif mode.contains('simulate')
    args += '-DJFM_ITER_SIMULATE'
elif mode.contains('gpu')
    args += '-DJFM_MODE_GPU'
    subdir('JFMServices/Calculator')
    deps += libcalculator_gpu_dep
endif

executable('JFM',
    sources: JFM_sources,
    include_directories: JFM_includes,
    cpp_args: ['-std=c++23',
               '-DJFM_PLATFORM_NIX',
               args],
    cuda_args: ['-std=c++20', '-gencode=arch=compute_86,code=sm_86', '-DJFM_COMPILER_CUDA'],
    link_args: [ linked_deps, '-lglfw', '-lGL'],
    dependencies: deps,
    install: true)
