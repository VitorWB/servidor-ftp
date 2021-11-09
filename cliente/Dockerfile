FROM gcc:4.9
COPY . /usr/src/cliente
WORKDIR /usr/src/cliente
RUN gcc cliente.cpp -o cliente
CMD ["./cliente"]