FROM gcc:9.4 AS builder
WORKDIR /app
COPY . .
RUN make

FROM debian:bookworm-slim
WORKDIR /app
RUN apt-get update && apt-get install -y libstdc++6 && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/dist /app
EXPOSE 4000
CMD ["./server"]
