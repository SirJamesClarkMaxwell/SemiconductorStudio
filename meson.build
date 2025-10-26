project('JFM', 'cpp',
    version: '1.0.0')

cpp = meson.get_compiler('cpp')

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
JFM_includes = []

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

args = []
if get_option('multi')
    args += '-DJFM_MULTITHREADED'
endif

executable('JFM',
    sources: JFM_sources,
    include_directories: JFM_includes,
    cpp_args: ['-std=c++23','-fexperimental-library',
               '-DJFM_PLATFORM_NIX',
               args,
               '-Wno-unused-variable', '-Wno-unused-function', '-Wno-unused-but-set-variable'],
    link_args: [ linked_deps, '-lglfw', '-lGL'],
    install: true)
