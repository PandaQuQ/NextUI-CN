#!/bin/sh

cd "$(dirname "$0")"
./airlock.elf > airlock.log 2>&1
