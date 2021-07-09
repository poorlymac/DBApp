FROM ubuntu:latest
RUN apt-get update && apt-get install -y gcc g++ mysql-client libmysqlclient-dev pkg-config xxd libgtk-3-dev libwebkit2gtk-4.0-dev
CMD ["/bin/bash"]