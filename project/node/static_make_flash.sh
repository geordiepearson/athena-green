if [[ $# -eq 1 ]] ; then
  echo "building with Mobile ID $1"
  sed -i "s/M_ID=./M_ID=$1/" CMakeLists.txt
else
  echo "building with Mobile ID 5"
  sed -i "s/M_ID=./M_ID=5/" CMakeLists.txt
fi

west build -p auto -b particle_argon  && west flash -r jlink

