#!/bin/bash

spack env create -d . spack.yaml
spack env activate -p .
spack install