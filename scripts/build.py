import argparse
import build_linux
import build_windows

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('--build-type', choices=['linux', 'windows'])
    parser.add_argument('--clean', action='store_true')
    parser.add_argument('--no-clean', dest='clean', action='store_false')
    parser.add_argument('--setup', action='store_true')
    parser.add_argument('--no-setup', dest='setup', action='store_false')
    parser.add_argument('--mode', choices=['single-core', 'multi-core', 'gpu', 'simulate'])
    parser.add_argument('--debug', action='store_true')
    args = parser.parse_args()

    print('Build JFM Application')
    if args.build_type == 'linux':
        print('\t target:\t linux\n')
        # NOTE: on linux example execution commands:
        #
        #   GPU device calculation mode:
        #        >  python3 scripts/build.py --build-type linux --clean --mode=gpu --setup
        #
        #   Multi CPU core calculation mode:
        #        >  python3 scripts/build.py --build-type linux --clean --mode=multi --setup
        #        >  python3 scripts/build.py --build-type linux --no-clean
        #
        if args.setup:
            build_linux.setup_dependencies(args.clean)
        build_linux.build(args.clean, args.mode, args.debug)
    else:
        print('\t target:\t windows\n')
        build_windows.build()
