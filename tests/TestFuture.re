open BsOspec.Cjs;

type timeoutId;
[@bs.val] [@bs.val] external setTimeout : ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

describe("Future", () => {

  let delay = (ms, f) => Future.make(resolve =>
    setTimeout(() => f() |> resolve, ms) |> ignore
  );

  test("sync chaining", () => {
    Future.value("one")
    |. Future.map(s => s ++ "!")
    |. Future.map(s => {
      s |. equals("one!");
    });
  });

  testAsync("async chaining", done_ => {
    delay(25, () => 20)
    |. Future.map(s => string_of_int(s))
    |. Future.map(s => s ++ "!")
    |. Future.get(s => {
      s |. equals("20!");
      done_();
    });
  });

  test("tap", () => {
    let v = ref(0);

    Future.value(99)
    |. Future.tap(n => v := n+1)
    |. Future.map(n => n - 9)
    |. Future.get(n => {
      n |. equals(90);
      v^ |. equals(100);
    });
  });

  test("flatMap", () => {
    Future.value(59)
    |. Future.flatMap(n => Future.value(n + 1))
    |. Future.get(n => {
      n |. equals(60);
    });
  });

  test("multiple gets", () => {
    let count = ref(0);
    let future = Future.make(resolve => {
      count := count^ + 1;
      resolve(count^);
    });
    count^ |. equals(1);

    future |. Future.get(c => {
      c |. equals(1);
    });
    count^ |. equals(1);

    future |. Future.get(c => {
      c |. equals(1);
    });
    count^ |. equals(1);
  });

  testAsync("multiple gets (async)", done_ => {
    let count = ref(0);
    let future = delay(25, () => 0)
    |. Future.map(_ => {
      count := count^ + 1;
    });

    count^ |. equals(~m="Callback is async", 0);

    future |. Future.get(_ => {
      count^ |. equals(~m="Runs after previous future", 1);
    });
    count^ |. equals(~m="Callback is async (2)", 0);

    future |. Future.get(_ => {
      count^ |. equals(~m="Previous future only runs once", 1);
    });
    count^ |. equals(0, ~m="Callback is async (3)");

    future |. Future.get(_ => done_());
  });

});
