m = module(project_name)

m.env["CCFLAGS"] += ["-g", "-D_LINUX_TARGET_", "-std=gnu++11" ]
m.env["CPPPATH"] += [os.path.join(os.environ['MIDWARE_DIR'],"include")]
m.env["LIBPATH"] += [os.path.join(os.environ['MIDWARE_DIR'],"target","tcc897x_64", "lib")]
m.env["LIBS"] += ["libJsoncpp"]
m.env["LIBS"] += ["libIPCCore"]
m.env["LIBS"] += ["libCommunicDevice"]
m.env["LIBS"] += ["libLogLib"]
m.env["LIBS"] += ["libBaseLib"]
m.binary_name = "hmi" + ".exe"

del m