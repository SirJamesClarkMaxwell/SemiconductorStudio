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
    parser.add_argument('--multi', action='store_true')
    args = parser.parse_args()

    print('Build JFM Application')
    if args.build_type == 'linux':
        print('\t target:\t linux\n')
        if args.setup:
            build_linux.setup_dependencies(args.clean, args.multi)
        build_linux.build(args.clean)
    else:
        print('\t target:\t windows\n')
        build_windows.build()
