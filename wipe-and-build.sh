#!/bin/bash

meson setup --wipe build
ninja -C build
