set -ex

#export GOOS=windows GOACH=amd64 CC=x86_64-w64-mingw32-gcc CXX=x86_64-w64-mingw32-g++ CGO_ENABLED=1

go get -d ./...

if [ "$CC" = "x86_64-w64-mingw32-gcc" ]; then
    echo $CC
else
    echo $CC
fi

mkdir -p ./bin
cd ./bin/

if [ "$CC" = "x86_64-w64-mingw32-gcc" ]; then
  go build -x -ldflags=all='-H windowsgui -s -w' ..
else
  go build -x ..
fi

cd -
