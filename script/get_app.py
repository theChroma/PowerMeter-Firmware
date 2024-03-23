Import("env")

def pre_fs_build(source, target, env):
    print("\r\nbefor buildfs\r\n")

env.AddPreAction("buildfs", pre_fs_build)
env.AddPreAction("uploadfs", pre_fs_build)