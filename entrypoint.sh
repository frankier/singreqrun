#!/usr/bin/env bash

if [[ -n "$TRACE" ]]; then
  set -o xtrace
fi

[ -z "$SIF_PATH" ] && echo "Point \$SIF_PATH at Singularity .sif file." && exit 1;
[ -f $SIF_PATH ] || (echo "Point \$SIF_PATH at Singularity .sif file." && exit 1);

DEFAULT_HOST_PROGS="sacct sacctmgr salloc sattach sbatch sbcast scancel scontrol \
scrontab sdiag sh5util sinfo sprio squeue sreport srun sshare sstat \
strigger sview singularity"

SCRIPTPATH="$( cd "$(dirname "$(readlink -f "$0")")" >/dev/null 2>&1 ; pwd -P )"

. $SCRIPTPATH/coordinator.sh

singularity exec \
    $SING_EXTRA_ARGS \
    --bind $SIF_PATH \
    --bind $tmp_dir \
    --bind $tmp_dir/req_run/:/var/run/req_run \
    $SIF_PATH $tmp_dir/patch_path_then.sh $tmp_dir/run_bootstrap.sh
