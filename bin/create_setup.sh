#!/bin/bash

echo Version number:
read version

tar -zcvf v-optimizer.linux.$version.tar.gz Linux
zip -r v-optimizer.window.$version.zip Windows
