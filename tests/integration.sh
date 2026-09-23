#!/bin/sh
set -eu

PORT=55151
SERVER="./bin/crypto-server"
CLIENT="./bin/crypto-client"

"$SERVER" "$PORT" >/tmp/socket-crypto-server.log 2>&1 &
SERVER_PID=$!

cleanup() {
  kill "$SERVER_PID" 2>/dev/null || true
  wait "$SERVER_PID" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

sleep 1

PLAINTEXT="HELLO WORLD"
KEY="XMCKL QWERTY"

CIPHERTEXT="$($CLIENT enc 127.0.0.1 "$PORT" "$PLAINTEXT" "$KEY")"
RECOVERED="$($CLIENT dec 127.0.0.1 "$PORT" "$CIPHERTEXT" "$KEY")"

if [ "$RECOVERED" != "$PLAINTEXT" ]; then
  echo "round-trip failed"
  echo "plaintext:  $PLAINTEXT"
  echo "ciphertext: $CIPHERTEXT"
  echo "recovered:  $RECOVERED"
  exit 1
fi

echo "integration test passed"
