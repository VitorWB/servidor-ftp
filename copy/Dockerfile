FROM gcc:4.9
COPY . /usr/src/copy-gcc
WORKDIR /usr/src/copy-gcc
RUN gcc copy.cpp -o copy
CMD ["./copy text.txt teste.txt"]