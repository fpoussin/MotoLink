#!/bin/sh

echo "Compiling PB files\n"

for i in `ls *.proto`; do
  echo $i
  protoc -I ../../nanopb/generator/proto -I. -o "${i%.*}.pb" $i;
  python ../../nanopb/generator/nanopb_generator.py "${i%.*}.pb"
  echo ""
done
