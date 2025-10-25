#!/bin/python3
import os
import subprocess
import threading

def setup_single_dep(d, abs_patch_path, doClean):
    dep = d[0]
    repo_link = d[1]
    commit_id = d[2]
    patches = d[3]

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

    os.chdir('..')
    print(f'{dep} done !')


def setup_dependencies(doClean):
    deps = [
        ['LambertW', 'https://github.com/SirJamesClarkMaxwell/LambertW', 'bf728a4',
            ['0001-feat-Add-build-script.patch']],
        ['NumericStorm', 'https://github.com/SirJamesClarkMaxwell/NumericStorm', 'f80187b',
            ['0001-fix-Make-compile-on-unix-systems.patch']],
        ['googletest', 'https://github.com/ArnoXX/googletest', 'ff233bd', []],
        ['imgui', 'https://github.com/ArnoXX/imgui', 'c795886',
            ['0001-fix-Make-compile-on-unix-systems.patch']],
        ['implot', 'https://github.com/ArnoXX/implot', '91fb380',
            ['0001-feat-Build-using-meson.patch']],
        ['yaml-cpp', 'https://github.com/ArnoXX/yaml-cpp', 'cbffe3b', []]
    ]
    patch_path = 'scripts/linux/patches'

    cwd = os.getcwd()

    print('Setting up dependencies')
    os.chdir('Vendor')

    if doClean:
        print('Cleaning up projects')

    tds = list()

    print('Fetching dependencies')
    for d in deps:
        tds.append( threading.Thread(target=setup_single_dep,
                                     args=(d, os.path.join(cwd, patch_path), doClean)) )

    for t in tds:
        t.start()
    for t in tds:
        t.join()

    os.chdir('..')
    print('Setup done !')


def build(doClean):
    def getMaxJobCount():
        import multiprocessing
        return (multiprocessing.cpu_count() * 2)

    buildDir = 'build'

    if doClean:
        subprocess.run(['rm', '-rf', buildDir])

    args = ['meson', buildDir]
    if os.path.exists(buildDir):
        args.append('--reconfigure')
    subprocess.run(args, env={'CC': 'clang',
                                             'CXX': 'clang++',
                                             'CXX_LD': 'lld',
                                             'CC_LD': 'lld'})
    subprocess.run(['ninja', '-C', buildDir, f'-j{getMaxJobCount()}'])

