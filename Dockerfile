FROM gcc:latest
COPY . /DBApp
WORKDIR /DBApp/
# This stops installs being prompted for Geography
RUN echo "tzdata tzdata/Areas select Australia" > /tmp/preseed.txt; \
    echo "tzdata tzdata/Zones/Australia select Sydney" >> /tmp/preseed.txt; \
    debconf-set-selections /tmp/preseed.txt && \
    apt-get update && \
    apt-get install -y tzdata
# Build dependancies
RUN apt-get update && apt-get install -y libmariadb-dev pkg-config xxd libgtk-3-dev libwebkit2gtk-4.0-dev fonts-noto-color-emoji
# Build
RUN bash build.sh
# Put a test config suitable to connect to a host mysql
RUN cp .DBApp.cnf ~
CMD ["./DBApp"]