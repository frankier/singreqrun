#!/bin/bash

wait_pipe() {
  until [ -p $1 ]
  do
       sleep 0.5
  done
}

wait_file() {
  until [ -f $1 ]
  do
       sleep 0.5
  done
}

rm -rf reqrun
mkdir -p reqrun
export REQRUN="$(pwd)/reqrun"

(
  echo 'hi this is stdin' | SINGREQRUN2_PREFIX=$REQRUN/ ./clients/unix/client.com foo bar baz
) &

wait_file $REQRUN/reqs
iden="$(tail -1 $REQRUN/reqs)"
echo "Pretending to run iden: $iden on behalf of container"

echo 'Command'
cat $REQRUN/$iden.cmd
echo ''

(
  (
    echo 'hi this is stdout' > $REQRUN/$iden.stdout
  ) &
  (
    echo 'hi this is stderr' > $REQRUN/$iden.stderr
  ) &
  (
    echo 'STDIN'
    tail -n +1 $REQRUN/$iden.stdin
    echo 'STDIN done'
  ) &
  wait 
)

echo '0' > $REQRUN/$iden.code
wait
