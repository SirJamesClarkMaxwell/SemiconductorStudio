import sys
import subprocess

def is_backtrace_log(l):
    backtrace_log = "[ INFO][599641] ../JFMServices/Helpers/utils.cpp:show_backtrace:56:"
    return l.find(backtrace_log) != -1

dump_start_tag = "[Dump][Start]"
def is_dump_start(l):
    return l.find(dump_start_tag) != -1

dump_end_tag = "[Dump][End]"
def is_dump_end(l):
    return  l.find(dump_end_tag) != -1

def is_dump_start_end(l):
    return is_dump_start(l) or is_dump_end(l)

def get_address(l):
    start = l.find("+")
    end = l.find(")")
    return l[start+1:end]

def get_callback_backtraces(dump):
    bts = list()
    bt = list()
    for l in dump:
        if is_dump_start(l):
            bt = [l]
        if is_dump_end(l):
            bt.append(l)
            bts.append(bt)
        else:
            bt.append(get_address(l))
    return bts

lines = [l.split('\n')[0] for l in open(sys.argv[1], 'r') if is_backtrace_log(l) or is_dump_start_end(l)]

callback_backtraces = get_callback_backtraces(lines)
BINARY_PATH = 'build/JFM'

for bt in callback_backtraces:
    print(dump_start_tag)
    for addr in bt[1:-1]:
        subprocess.run(['addr2line', '-f', '-C', '-e', BINARY_PATH, addr])
    print(dump_end_tag)

