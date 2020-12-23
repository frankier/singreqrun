class AbsoluteImport:
    def __enter__(self):
        import sys
        self.removed_sys_path = False
        if sys.path[0] == "":
            self.removed_sys_path = True
            del sys.path[0]
        self.saved_subprocess = None
        if "subprocess" in sys.modules:
            self.saved_subprocess = sys.modules["subprocess"]

    def __exit__(self, exc_type, exc_val, exc_tb):
        import sys
        if self.removed_sys_path:
            sys.path.insert(0, "")
        if self.saved_subprocess is not None:
            sys.modules["subprocess"] = self.saved_subprocess


import importlib
with AbsoluteImport():
    subprocess_spec = importlib.util.find_spec("subprocess")
subprocess = importlib.util.module_from_spec(subprocess_spec)
subprocess_spec.loader.exec_module(subprocess)
cur_mod = globals()
for k, v in vars(subprocess):
    if k == "check_output":
        continue
    cur_mod[k] = v


from req_run import check_output
