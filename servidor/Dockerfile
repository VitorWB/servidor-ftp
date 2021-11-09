FROM gcc:4.9
COPY . /usr/src/servidor
WORKDIR /usr/src/servidor
RUN gcc servidor.cpp -o servidor
CMD ["./servidor"]