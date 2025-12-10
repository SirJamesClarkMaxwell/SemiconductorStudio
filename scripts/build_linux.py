#!/bin/python3
import os
import subprocess
import threading

def getMaxJobCount():
    import multiprocessing
    return (multiprocessing.cpu_count() * 2)


def do_build(args, buildDir, builder, builder_env={}):
    print(args)
    subprocess.run(args, env={'CC': 'clang',
                              'CXX': 'clang++',
                              'CXX_LD': 'lld',
                              'CC_LD': 'lld'} | builder_env)
    subprocess.run([builder, '-C', buildDir, f'-j{getMaxJobCount()}'])


def do_build_yaml_cpp(buildDir='build'):
    args = ['cmake', '-B', buildDir]
    do_build(args, buildDir, 'make', builder_env={'PATH': '/usr/bin'})


def do_meson_build(doClean=False, buildDir='build', mode=None, debug=False):
    args = ['meson', buildDir]
    modes = ['single-core', 'multi-core', 'gpu', 'simulate']

    if os.path.exists(buildDir):
        args.append('--reconfigure')
    if mode in modes:
        args.append(f'-Dmode={mode}')
    if debug:
        args.append('-Ddebug=true')

    do_build(args, buildDir, 'ninja')


def setup_single_dep(d, abs_patch_path, doClean):
    dep = d[0]
    repo_link = d[1]
    commit_id = d[2]
    patches = d[3]
    build_cb = d[4]

    if doClean:
        subprocess.run(['rm', '-rf', dep])

    print(f'Setting up {dep} ...')
    subprocess.run(['git', 'clone', repo_link], capture_output=True)

    os.chdir(dep)
    subprocess.run(['git', 'checkout', commit_id], capture_output=True)

    if len(patches) > 0:
        print('Applying required patches ..')
        for patch in patches:
            # FIXME: make sure not to fail here !
            # at least check potential errors
            subprocess.run(['git', 'am', '--keep', os.path.join(abs_patch_path, dep, patch)],
                           capture_output=False)

    if build_cb is not None:
        build_cb()

    os.chdir('..')
    print(f'{dep} done !')


def setup_dependencies(doClean):
    deps = [
        ['LambertW', 'https://github.com/SirJamesClarkMaxwell/LambertW', 'bf728a4',
            ['0001-feat-Add-build-script.patch'], do_meson_build],
        ['NumericStorm', 'https://github.com/SirJamesClarkMaxwell/NumericStorm', 'dea5c27', [], None],
        ['googletest', 'https://github.com/ArnoXX/googletest', 'ff233bd', [], None],
        ['imgui', 'https://github.com/ArnoXX/imgui', 'c795886',
            ['0001-fix-Make-compile-on-unix-systems.patch'], do_meson_build],
        ['implot', 'https://github.com/ArnoXX/implot', '91fb380',
            ['0001-feat-Build-using-meson.patch'], do_meson_build],
        ['yaml-cpp', 'https://github.com/ArnoXX/yaml-cpp', 'cbffe3b', [], do_build_yaml_cpp]
    ]
    patch_path = 'scripts/linux/patches'

    cwd = os.getcwd()

    print('Setting up dependencies')
    os.chdir('Vendor')

    if doClean:
        print('Cleaning up projects')


    print('Fetching dependencies')
    for d in deps:
        setup_single_dep(d, os.path.join(cwd, patch_path), doClean)

    os.chdir('..')
    print('Setup done !')


def build(doClean, mode, debug):
    buildDir = 'build'

    if doClean:
        subprocess.run(['rm', '-rf', buildDir])

    do_meson_build(doClean, mode=mode, debug=debug)

