if [[ $# -eq 1 ]] ; then
  echo "building with Mobile ID $1"
  sed -i "s/M_ID=./M_ID=$1/" CMakeLists.txt
else
  echo "building with Mobile ID 1"
  sed -i "s/M_ID=./M_ID=1/" CMakeLists.txt
fi
west build -b thingy52_nrf52832 -p auto -- -DMOBILE_NODE=ON && west flash -r jlink
