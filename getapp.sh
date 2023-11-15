#!/bin/bash

app_dir="C:/dev/PowerMeter-App"
firmware_dir="C:/Users/Samuel/Documents/PlatformIO/Projects/WebServer"

target_dir="${firmware_dir}/data/app"

echo "Building flutter project at ${app_dir} for the web"
cd $app_dir
flutter build web --web-renderer html


echo "Copying App"
rm -f --recursive $target_dir
mkdir -p $target_dir
cp -a "${app_dir}/build/web/." $target_dir

echo "Deleting uneseccary files"
rm -f "${target_dir}/assets/NOTICES"

echo "Compressing"
gzip -r "${target_dir}/"