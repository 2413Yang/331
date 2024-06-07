dependencies = ["kzjs", "kzui"]

m = module(project_name)

if build_from_sources:
    m.depends += dependencies
else:
    m.used_libraries += dependencies

m.root = os.path.join("..", "..", "..", "src")

# m.used_libraries += ["v8"]
m.env["CPPDEFINES"] += ["KANZI_V8_API="]

del m