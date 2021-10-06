#!/usr/bin/env bash

DEFAULT_HOST_PROGS="sacct sacctmgr salloc sattach sbatch sbcast scancel scontrol \
scrontab sdiag sh5util sinfo sprio squeue sreport srun sshare sstat \
strigger sview singularity"

SCRIPTPATH="$( cd "$(dirname "$(readlink -f "$0")")" >/dev/null 2>&1 ; pwd -P )"

if [[ -n "$TRACE" ]]; then
  set -o xtrace
fi

[ -z "$SIF_PATH" ] && echo "Point \$SIF_PATH at Singularity .sif file." && exit 1;
[ -f $SIF_PATH ] || (echo "Point \$SIF_PATH at Singularity .sif file." && exit 1);

# Step 1) Setup tmp dir with shared files
tmp_dir=$(mktemp -d -t reqrun-XXXXXXXXXX)
pushd $tmp_dir

. $SCRIPTPATH/setup.sh

mkdir bin
cd bin

if [ -z ${HOST_PROGS+x} ]; then
  HOST_PROGS="$DEFAULT_HOST_PROGS"
fi

for prog in $HOST_PROGS; do
  ln -s $SCRIPTPATH/client.com $prog
done

cd ..

# Step 2) Copy in executor script, which listens for req_run
cp $SCRIPTPATH/executor.sh $SCRIPTPATH/run_initial_bootstrap.sh .
chmod +x executor.sh

# Step 3) Copy in user bootstrap which is run inside container
TMP_DIR="$tmp_dir" \
ARGS="$@" \
envsubst \
< $SCRIPTPATH/run_bootstrap.sh \
> run_bootstrap.sh

chmod +x run_bootstrap.sh

# Step 4)
# Execute coordinator using Singularity
# Must map in bootstrapped tmp directory with bootstrap
# script and its requirements
popd

mkdir -p $tmp_dir/req_run
touch $tmp_dir/req_run/reqs

trap "exit" INT TERM
trap "kill 0" EXIT

tail -f $tmp_dir/req_run/reqs 2>/dev/null | $tmp_dir/executor.sh $tmp_dir &

if [[ -n "$PRE_COORDINATOR_SCRIPT" ]]; then
  eval "$PRE_COORDINATOR_SCRIPT"
fi

singularity exec \
    $SING_EXTRA_ARGS \
    --bind $SIF_PATH \
    --bind $tmp_dir \
    --bind $tmp_dir/req_run/:/var/run/req_run \
    --bind $tmp_dir/bin/:/bin \
    $SIF_PATH $tmp_dir/run_initial_bootstrap.sh
