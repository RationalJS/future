open BsOspec.Cjs;
exception TestError(string);

type timeoutId;
[@bs.val] [@bs.val]
external setTimeout: ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

describe("Future", () => {
  let delay = (ms, f) =>
    Future.make(resolve => setTimeout(() => f() |> resolve, ms) |> ignore);

  test("sync chaining", () =>
    Future.value("one")
    ->(Future.map(s => s ++ "!"))
    ->(Future.map(s => s->(equals("one!"))))
  );

  testAsync("async chaining", done_ =>
    delay(25, () => 20)
    ->(Future.map(s => string_of_int(s)))
    ->(Future.map(s => s ++ "!"))
    ->(
        Future.get(s => {
          s->(equals("20!"));
          done_();
        })
      )
  );

  test("tap", () => {
    let v = ref(0);

    Future.value(99)
    ->(Future.tap(n => v := n + 1))
    ->(Future.map(n => n - 9))
    ->(
        Future.get(n => {
          n->(equals(90));
          (v^)->(equals(100));
        })
      );
  });

  test("flatMap", () =>
    Future.value(59)
    ->(Future.flatMap(n => Future.value(n + 1)))
    ->(Future.get(n => n->(equals(60))))
  );

  test("multiple gets", () => {
    let count = ref(0);
    let future =
      Future.make(resolve => {
        count := count^ + 1;
        resolve(count^);
      });
    (count^)->(equals(1));

    future->(Future.get(c => c->(equals(1))));
    (count^)->(equals(1));

    future->(Future.get(c => c->(equals(1))));
    (count^)->(equals(1));
  });

  testAsync("multiple gets (async)", done_ => {
    let count = ref(0);
    let future = delay(25, () => 0)->(Future.map(_ => count := count^ + 1));

    (count^)->(equals(~m="Callback is async", 0));

    future->(
              Future.get(_ =>
                (count^)->(equals(~m="Runs after previous future", 1))
              )
            );
    (count^)->(equals(~m="Callback is async (2)", 0));

    future->(
              Future.get(_ =>
                (count^)->(equals(~m="Previous future only runs once", 1))
              )
            );
    (count^)->(equals(0, ~m="Callback is async (3)"));

    future->(Future.get(_ => done_()));
  });

  testAsync("all (async)", done_ => {
    let future =
      Future.all([Future.value(1), delay(25, () => 2), delay(50, () => 3)]);

    future->Future.get(result => {
      /* Not sure how to get this assertion library to respect immutable lists, so just do this */
      if (result != [1, 2, 3]) {
        raise(TestError("Expected [1, 2, 3]"));
      };
      done_();
    });
  });
});

describe("Future Belt.Result", () => {
  test("mapOk", () => {
    Belt.Result.Ok("two")
    ->Future.value
    ->(Future.mapOk(s => s ++ "!"))
    ->(Future.get(r => Belt.Result.getExn(r)->(equals("two!"))));

    Belt.Result.Error("err2")
    ->Future.value
    ->(Future.mapOk(s => s ++ "!"))
    ->(
        Future.get(r =>
          switch (r) {
          | Ok(_) => raise(TestError("shouldn't be possible"))
          | Error(e) => e->(equals("err2"))
          }
        )
      );
  });

  test("mapError", () => {
    Belt.Result.Ok("three")
    ->Future.value
    ->(Future.mapError(s => s ++ "!"))
    ->(Future.get(r => Belt.Result.getExn(r)->(equals("three"))));

    Belt.Result.Error("err3")
    ->Future.value
    ->(Future.mapError(s => s ++ "!"))
    ->(
        Future.get(r =>
          switch (r) {
          | Ok(_) => raise(TestError("shouldn't be possible"))
          | Error(e) => e->(equals("err3!"))
          }
        )
      );
  });

  test("flatMapOk", () => {
    Belt.Result.Ok("four")
    ->Future.value
    ->(Future.flatMapOk(s => Belt.Result.Ok(s ++ "!")->Future.value))
    ->(Future.get(r => Belt.Result.getExn(r)->(equals("four!"))));

    Belt.Result.Error("err4.1")
    ->Future.value
    ->(Future.flatMapOk(s => Belt.Result.Ok(s ++ "!")->Future.value))
    ->(
        Future.get(r =>
          switch (r) {
          | Ok(_) => raise(TestError("shouldn't be possible"))
          | Error(e) => e->(equals("err4.1"))
          }
        )
      );

    Belt.Result.Error("err4")
    ->Future.value
    ->(Future.flatMapError(e => Belt.Result.Error(e ++ "!")->Future.value))
    ->(
        Future.get(r =>
          switch (r) {
          | Ok(_) => raise(TestError("shouldn't be possible"))
          | Error(e) => e->(equals("err4!"))
          }
        )
      );
  });

  test("flatMapError", () => {
    Belt.Result.Ok("five")
    ->Future.value
    ->(Future.flatMapError(s => Belt.Result.Error(s ++ "!")->Future.value))
    ->(Future.get(r => Belt.Result.getExn(r)->(equals("five"))));

    Belt.Result.Error("err5")
    ->Future.value
    ->(Future.flatMapError(e => Belt.Result.Error(e ++ "!")->Future.value))
    ->(
        Future.get(r =>
          switch (r) {
          | Ok(_) => raise(TestError("shouldn't be possible"))
          | Error(e) => e->(equals("err5!"))
          }
        )
      );
  });

  test("tapOk", () => {
    let x = ref(1);
    let y = ref(1);

    Belt.Result.Ok(10)
    ->Future.value
    ->(Future.tapOk(n => x := x^ + n))
    ->(Future.get(_ => (x^)->(equals(11))));

    Belt.Result.Error(10)
    ->Future.value
    ->(Future.tapOk(n => y := y^ + n))
    ->(Future.get(_ => (y^)->(equals(1))));
  });

  test("tapError", () => {
    let x = ref(1);
    let y = ref(1);

    Belt.Result.Ok(10)
    ->Future.value
    ->(Future.tapError(n => x := x^ + n))
    ->(Future.get(_ => (x^)->(equals(1))));

    Belt.Result.Error(10)
    ->Future.value
    ->(Future.tapError(n => y := y^ + n))
    ->(Future.get(_ => (y^)->(equals(11))));
  });
});