FROM debian:stable-20231030-slim

# Set package manager
ENV PKG_MGR=apt
ENV PKG_MGR_INSTALL="$PKG_MGR install -y"

RUN $PKG_MGR update && $PKG_MGR upgrade

# Install binary dependencies
RUN $PKG_MGR_INSTALL make
RUN $PKG_MGR_INSTALL gcc
RUN $PKG_MGR_INSTALL g++
RUN $PKG_MGR_INSTALL libc-dev
RUN $PKG_MGR_INSTALL libstdc++6
RUN $PKG_MGR_INSTALL libncurses5-dev

# Install script dependencies
RUN $PKG_MGR_INSTALL bash
RUN $PKG_MGR_INSTALL curl

# Build executable
ENV DEST=/build/
WORKDIR $DEST
COPY ./*.sh $DEST
COPY ./*.txt $DEST
COPY ./*.csv $DEST
COPY ./*.cpp $DEST
COPY ./*.tpp $DEST
COPY ./*.hpp $DEST

RUN ./configure.sh --with-prefix=/usr
RUN make -j5
RUN make install

# Set up command environment
ENV W3M_USER_AGENT="Lynx/2.8.9rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/1.1.1d"
ENV WWW_HOME="https://lite.duckduckgo.com/lite/"
CMD [ "ls" ]
