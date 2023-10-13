FROM alpine:3.18.4
RUN apk update && apk upgrade

# Install binary dependencies
RUN apk add make
RUN apk add gcc
RUN apk add g++
RUN apk add libc-dev
RUN apk add libstdc++
RUN apk add ncurses ncurses-dev libncursesw libncurses++

# Install script dependencies
RUN apk add bash
RUN apk add curl

# Build executable
ENV DEST=/build/
WORKDIR $DEST
COPY ./*.sh $DEST
COPY ./*.txt $DEST
COPY ./*.csv $DEST
COPY ./*.cpp $DEST
COPY ./*.tpp $DEST
COPY ./*.hpp $DEST

RUN echo Listing files ...
RUN ls

RUN echo Finding fork ...
RUN grep -R -e fork /usr/include

RUN ./configure.sh --with-prefix=/tmp
RUN make -j5

# Set up command environment
ENV W3M_USER_AGENT="Lynx/2.8.9rel.1 libwww-FM/2.14 SSL-MM/1.4.1 OpenSSL/1.1.1d"
CMD [ "ls" ]
