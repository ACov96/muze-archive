#!/bin/sh

for f in test/*
do
  echo "================================="
  echo "== Testing $f"
  echo "================================="
  ./muzec $f
done

