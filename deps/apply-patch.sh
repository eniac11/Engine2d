#!/usr/bin/sh

PATCH_FILE=${PATCH_FILE}

if [ -z $PATCH_FILE ]; then
    exit 1
fi

git apply $PATCH_FILE || true
