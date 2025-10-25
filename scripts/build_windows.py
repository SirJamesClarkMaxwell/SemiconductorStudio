import subprocess

def build():
    subprocess.run(("./premake5.exe", "vs2022"))
    pass
