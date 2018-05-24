open BsOspec.Cjs;
exception TestError(string);

type timeoutId;
[@bs.val] [@bs.val] external setTimeout : ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

exception Err(string);

describe("Future", () => {

  let delay = (ms, f) => Future.make((resolve, _reject) =>
    setTimeout(() => f() |> resolve, ms) |> ignore
  );

  let delayError = (ms, f) => Future.make((_resolve, reject) =>
    setTimeout(() => f() |> reject, ms) |> ignore
  );

  test("sync chaining", () => {
    Future.value("one")
    |. Future.map(s => s ++ "!")
    |. Future.get(
      s => {
        s |. equals("one!");
      },
      _ => {
        1 |. equals(2);
      }
    )
  });

  test("sync error chaining", () => {
    Future.error(Err("one"))
    |. Future.catch(e => switch (e) {
      | Err(s) => Err(s ++ "!") |. Future.error
      | _ => assert false
    })
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      err => {
        err |. deepEquals(Err("one!"));
      }
    )
  });

  testAsync("async chaining", done_ => {
    delay(25, () => 20)
    |. Future.map(s => string_of_int(s))
    |. Future.map(s => s ++ "!")
    |. Future.get(
      s => {
        s |. equals("20!");
        done_();
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  testAsync("async error chaining", done_ => {
    delayError(25, () => Err("20"))
    |. Future.catch(err => switch (err) {
      | Err(s) => Err(s ++ "!") |. Future.error
      | _ => assert false
    })
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      error => {
        error |. deepEquals(Err("20!"));
        done_();
      }
    );
  });

  test("tap", () => {
    let v = ref(0);

    Future.value(99)
    |. Future.tap(n => v := n+1)
    |. Future.map(n => n - 9)
    |. Future.get(
      n => {
        n |. equals(90);
        v^ |. equals(100);
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  test("flatMap", () => {
    Future.value(59)
    |. Future.flatMap(n => Future.value(n + 1))
    |. Future.get(
      n => {
        n |. equals(60);
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  test("raising resolver", () => {
    Future.make((_resolve, _reject) => raise(Err("one")))
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      e => {
        e |. deepEquals(Err("one"));
      }
    )
  });

  testAsync("raising map", done_ => {
    delay(20, () => 59)
    |. Future.map(_ => raise(Err("one")))
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      e => {
        e |. deepEquals(Err("one"));
        done_();
      }
    )
  });

  testAsync("raising flatMap", done_ => {
    delay(20, () => 59)
    |. Future.flatMap(_ => raise(Err("one")))
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      e => {
        e |. deepEquals(Err("one"));
        done_();
      }
    )
  });

  testAsync("raising tap", done_ => {
    delay(20, () => 59)
    |. Future.tap(_ => raise(Err("one")))
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      e => {
        e |. deepEquals(Err("one"));
        done_();
      }
    )
  });

  testAsync("raising catch", done_ => {
    delay(20, () => 59)
    |. Future.tap(_ => assert false)
    |. Future.catch(_ => raise(Err("one")))
    |. Future.get(
      _ => {
        1 |. equals(2);
      },
      e => {
        e |. deepEquals(Err("one"));
        done_();
      }
    )
  });

  test("multiple gets", () => {
    let count = ref(0);
    let future = Future.make((resolve, _reject) => {
      count := count^ + 1;
      resolve(count^);
    });
    count^ |. equals(1);

    future |. Future.get(
      c => {
        c |. equals(1);
      },
      _ => {
        1 |. equals(2);
      }
    );
    count^ |. equals(1);

    future |. Future.get(
      c => {
        c |. equals(1);
      },
      _ => {
        1 |. equals(2);
      }
    );
    count^ |. equals(1);
  });

  testAsync("multiple gets (async)", done_ => {
    let count = ref(0);
    let future = delay(25, () => 0)
    |. Future.map(_ => {
      count := count^ + 1;
    });

    count^ |. equals(~m="Callback is async", 0);

    future |. Future.get(
      _ => {
        count^ |. equals(~m="Runs after previous future", 1);
      },
      _ => {
        1 |. equals(2);
      }
    );
    count^ |. equals(~m="Callback is async (2)", 0);

    future |. Future.get(
      _ => {
        count^ |. equals(~m="Previous future only runs once", 1);
      },
      _ => {
        1 |. equals(2);
      }
    );
    count^ |. equals(0, ~m="Callback is async (3)");

    future |. Future.get(_ => done_(), _ => ());
  });

});


describe("Future Belt.Result", () => {

  test("mapOk", () => {
    Belt.Result.Ok("two")
    |. Future.value
    |. Future.mapOk(s => s ++ "!")
    |. Future.get(
      r => {
        Belt.Result.getExn(r) |. equals("two!");
      },
      _ => {
        1 |. equals(2);
      }
    );

    Belt.Result.Error("err2")
    |. Future.value
    |. Future.mapOk(s => s ++ "!")
    |. Future.get(
      r => switch (r) {
        | Ok(_) => raise(TestError("shouldn't be possible"))
        | Error(e) => e |. equals("err2");
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  test("mapError", () => {
    Belt.Result.Ok("three")
    |. Future.value
    |. Future.mapError(s => s ++ "!")
    |. Future.get(
      r => {
        Belt.Result.getExn(r) |. equals("three");
      },
      _ => {
        1 |. equals(2);
      }
    );

    Belt.Result.Error("err3")
    |. Future.value
    |. Future.mapError(s => s ++ "!")
    |. Future.get(
      r => switch (r) {
        | Ok(_) => raise(TestError("shouldn't be possible"))
        | Error(e) => e |. equals("err3!");
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  test("tapOk", () => {
    let x = ref(1);
    let y = ref(1);

    Belt.Result.Ok(10)
    |. Future.value
    |. Future.tapOk(n => x := x^ + n)
    |. Future.get(
      _ => {
        x^ |. equals(11);
      },
      _ => {
        1 |. equals(2);
      }
    );

    Belt.Result.Error(10)
    |. Future.value
    |. Future.tapOk(n => y := y^ + n)
    |. Future.get(
      _ => {
        y^ |. equals(1);
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

  test("tapError", () => {
    let x = ref(1);
    let y = ref(1);

    Belt.Result.Ok(10)
    |. Future.value
    |. Future.tapError(n => x := x^ + n)
    |. Future.get(
      _ => {
        x^ |. equals(1);
      },
      _ => {
        1 |. equals(2);
      }
    );

    Belt.Result.Error(10)
    |. Future.value
    |. Future.tapError(n => y := y^ + n)
    |. Future.get(
      _ => {
        y^ |. equals(11);
      },
      _ => {
        1 |. equals(2);
      }
    );
  });

});
