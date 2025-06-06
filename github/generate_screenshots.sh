#!/bin/bash

pushd "$(dirname "$0")" >> /dev/null

files=$(ls *.png)
screenshotText="# NextUI 中文版截图\n\n"

for file in $files; do
    screenshotText+="<img src=\"$file\" width=320 /> "
done

echo -e $screenshotText > README.md
popd >> /dev/null