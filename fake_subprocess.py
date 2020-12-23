class AbsoluteImport:
    def __enter__(self):
        import sys
        from os.path import samefile
        self.saved_sys_path = sys.path[:]
        to_rm_idxs = []
        for idx, path in enumerate(sys.path):
            if path == "":
                path = "."
            try:
                if samefile(path, "."):
                    to_rm_idxs.append(idx)
            except FileNotFoundError:
                pass
        to_rm_idxs.sort(reverse=True)
        for rm_idx in to_rm_idxs:
            del sys.path[rm_idx]
        self.saved_subprocess = None
        if "subprocess" in sys.modules:
            self.saved_subprocess = sys.modules["subprocess"]
            del sys.modules["subprocess"]

    def __exit__(self, exc_type, exc_val, exc_tb):
        import sys
        sys.path = self.saved_sys_path
        if self.saved_subprocess is not None:
            sys.modules["subprocess"] = self.saved_subprocess


import importlib
with AbsoluteImport():
    subprocess_spec = importlib.util.find_spec("subprocess")
subprocess = importlib.util.module_from_spec(subprocess_spec)
subprocess_spec.loader.exec_module(subprocess)
cur_mod = globals()
for k, v in vars(subprocess).items():
    if k == "check_output":
        continue
    cur_mod[k] = v


from req_run import check_output
