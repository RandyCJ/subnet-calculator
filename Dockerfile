FROM gcc:9.4.0
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o server server.c -lm
CMD ["./server"]