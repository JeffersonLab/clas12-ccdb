#!/bin/bash

cd python
find . -type f | grep -v '*.pyc' | xargs zip -r ../ccdb.zip
cd ..
echo '#!/usr/bin/env python' | cat - ccdb.zip > ccdb
chmod +x ccdb
rm ccdb.zip
