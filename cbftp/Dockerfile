FROM alpine
ENV PATH="/app/bin:${PATH}"
EXPOSE 55477/tcp
EXPOSE 55477/udp
RUN apk add g++ make openssl-dev ncurses-dev
COPY . /app
RUN make -C /app -j$(nproc)
CMD ["cbftp"]
