# Build in container
docker build -t dbapp .
if [ $? -ne 0 ]
then
    exit
fi
# Copy across to host
rm -f DBApp
docker run -v $(pwd):/mnt dbapp:latest cp ./DBApp /mnt
if [ $? -ne 0 ]
then
    exit
fi
ls -la DBApp
# Test run
open /Applications/Utilities/XQuartz.app
xhost + 127.0.0.1
docker run -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=host.docker.internal:0 dbapp:latest
