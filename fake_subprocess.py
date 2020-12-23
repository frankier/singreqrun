class AbsoluteImport:
    def __enter__(self):
        import sys
        self.removed_sys_path = False
        if sys.path[0] == "":
            self.removed_sys_path = True
            del sys.path[0]

    def __exit__(self, exc_type, exc_val, exc_tb):
        import sys
        if self.removed_sys_path:
            sys.path.insert(0, "")


with AbsoluteImport():
    from subprocess import *


def check_output(args):
    from req_run import check_output
    with AbsoluteImport():
        return check_output(args)
