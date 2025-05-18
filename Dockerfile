FROM gcc:9.4 AS builder
WORKDIR /app
COPY ./server ./server
COPY Makefile .
RUN make server

FROM debian:bookworm-slim
RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/dist /app
EXPOSE 4000
CMD ["/app/server"]