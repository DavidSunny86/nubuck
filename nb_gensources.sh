# nb_gensources.sh
# generates sources.cmake containing all c++ files in a directory
# usage:
# find $(cat whitelist.txt) -type -d -exec ./gensources.sh {} \;

shopt -s nullglob
shopt -s extglob

curdir=$pwd

cd $1

echo "recurse()" >> sources.cmake
echo "add_sources(" >> sources.cmake
for f in @(*.cpp|*.c|*.h|*.ui); do
    echo "    "$f >> sources.cmake
done
echo ")" >> sources.cmake

cd $curdir

