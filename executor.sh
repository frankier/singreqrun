#!/usr/bin/env bash

tmp_dir=$1

wait_pipe() {
  until [ -p $1 ]
  do
       sleep 0.5
  done
}

if [[ -z "$NO_CLEANUP" ]]; then
  trap "exit" INT TERM
  trap "rm -rf $tmp_dir" EXIT
fi

while IFS= read -r iden
do
  if [[ -n "$EXECUTOR_TRACE" ]]; then
    echo "Running iden: $iden on behalf of container: $cmd"
  fi
  if [[ -n "$FILE_RPC" ]]; then
    $tmp_dir/req_run/$iden.cmd \
      > $tmp_dir/req_run/$iden.stdout \
      2> $tmp_dir/req_run/$iden.stderr
    echo $? > $tmp_dir/req_run/$iden.code
    if [[ -n "$EXECUTOR_TRACE" ]]; then
      cat $tmp_dir/req_run/$iden.stdout
      cat $tmp_dir/req_run/$iden.stderr
    fi
  else
    wait_pipe $tmp_dir/req_run/$iden.stdin
    wait_pipe $tmp_dir/req_run/$iden.stdout
    wait_pipe $tmp_dir/req_run/$iden.stderr
    $tmp_dir/req_run/$iden.cmd \
      < $tmp_dir/req_run/$iden.stdin \
      > $tmp_dir/req_run/$iden.stdout \
      2> $tmp_dir/req_run/$iden.stderr
    echo $? > $tmp_dir/req_run/$iden.code
  fi
done
