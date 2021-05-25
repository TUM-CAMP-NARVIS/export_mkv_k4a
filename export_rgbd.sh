if test "$#" -ne 3; then
    echo "Illegal number of parameters"
    echo "Usage: ./extract_rgbd.sh {recording_base_directory} {first_frame} {last_frame}"
    exit
fi

directory=$1

trap 'jobs -p | xargs kill' INT

for f in cn01 cn02 cn03 cn04 cn05 cn06
do
  mkdir -p ./tmp_data/${f}/
  ./build/bin/extract_mkv_k4a --first-frame $2 --last-frame $3 $directory/${f}/capture.mkv --output-dir /data/test_data/holistic_or/tmp/${f} --color --rgbd &
done

for f in cn01 cn02 cn03 cn04 cn05 cn06
do
   mkdir -p ./tmp_data/${f}/
   ./build/bin/extract_mkv_k4a --first-frame $(($3 + 1)) --last-frame $(($3 + $3 - $2)) $directory/${f}/capture.mkv --output-dir /data/test_data/holistic_or/tmp/${f} --color --rgbd &
 done

wait
