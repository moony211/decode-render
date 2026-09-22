#include <asio.hpp>
#include <asio/experimental/concurrent_channel.hpp>
#include <cstdio>

using asio::experimental::concurrent_channel;
using Chan = concurrent_channel<void(asio::error_code, int)>;

asio::awaitable<void> producer(Chan &ch) {
  for (int i = 0; i < 5; ++i) {
    // async_send는 채널 시그니처의 Args...(error_code, int)를 그대로 받고,
    // 완료 시그니처는 void(error_code)이므로 as_tuple 결과는 1-튜플이다.
    auto [ec] = co_await ch.async_send(asio::error_code{}, i,
                                       asio::as_tuple(asio::use_awaitable));
    if (ec)
      co_return; // close/cancel 감지
  }
  ch.close();
}
asio::awaitable<void> consumer(Chan &ch) {
  for (;;) {
    auto [ec, v] =
        co_await ch.async_receive(asio::as_tuple(asio::use_awaitable));
    if (ec)
      break; // close 감지 = EOF 프로토콜 예행연습
    std::printf("recv %d\n", v);
  }
}
int main() {
  asio::io_context ctx;
  Chan ch(ctx.get_executor(), 2); // 용량 2 → 백프레셔 동작 포함
  asio::co_spawn(ctx, producer(ch), asio::detached);
  asio::co_spawn(ctx, consumer(ch), asio::detached);
  ctx.run();
}