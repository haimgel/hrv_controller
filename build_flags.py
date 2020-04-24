import subprocess
Import("env")

gitref = subprocess.run(['git', 'rev-parse', '--short=8', 'HEAD'], stdout=subprocess.PIPE).stdout.decode('utf-8').strip()
print("Building gitref: ", gitref)
env.Append(CPPDEFINES=[('PIO_SRC_REV', f'\\"{gitref}\\"')])
