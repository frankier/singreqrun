# Step 1) Setup tmp dir with shared files
tmp_dir=$(mktemp -d -t reqrun-XXXXXXXXXX)
pushd $tmp_dir

HOSTBIN=$tmp_dir/bin

if [[ -z "$NO_COORDINATOR_SETUP" ]]; then
  . $SCRIPTPATH/setup.sh
fi

mkdir bin
cd bin

if [ -z ${HOST_PROGS+x} ]; then
  HOST_PROGS="$DEFAULT_HOST_PROGS"
fi

for prog in $HOST_PROGS; do
  ln -s $SCRIPTPATH/client.com $prog
done

if [[ -n "$HOST_PROGS" ]]; then
  # We run the APE client on the host if need be so it gets specialised since
  # Python can't handle the bash shim
  SRR_CLIENT_NOOP=1 $SCRIPTPATH/client.com
fi

cd ..

# Step 2) Copy in executor script, which listens for req_run
cp $SCRIPTPATH/executor.sh $SCRIPTPATH/patch_path_then.sh .
chmod +x executor.sh patch_path_then.sh

# Pre 3) Save args preserving quotes
ARGS=''
for i in "$@"; do
    case "$i" in
        *\'*)
            i=`printf "%s" "$i" | sed "s/'/'\"'\"'/g"`
            ;;
        *) : ;;
    esac
    ARGS="$ARGS '$i'"
done

# Step 3) Copy in user bootstrap which is run inside container
TMP_DIR="$tmp_dir" \
ARGS="$ARGS" \
DOLLAR='$' \
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

if [[ -n "$FILE_RPC" ]]; then
  touch $tmp_dir/req_run/reqs
else
  mkfifo $tmp_dir/req_run/reqs
fi

trap "exit" INT TERM
trap "kill 0" EXIT

tail -f $tmp_dir/req_run/reqs 2>/dev/null | $tmp_dir/executor.sh $tmp_dir &

if [[ -n "$PRE_COORDINATOR_SCRIPT" ]]; then
  eval "$PRE_COORDINATOR_SCRIPT"
fi

SING_EXTRA_ARGS="$SING_EXTRA_ARGS --bind $HOSTBIN:/hostbin"

if [[ -n "$FILE_RPC" ]]; then
  SING_EXTRA_ARGS="$SING_EXTRA_ARGS --env SINGREQRUN2_SYNC_RPC=1"
fi
