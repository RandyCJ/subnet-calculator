FROM gcc:9.4.0
COPY . /usr/src/myapp
WORKDIR /usr/src/myapp
RUN gcc -o echo_server echo_server.c -lm
CMD ["./echo_server"]