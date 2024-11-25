open Jest
open Expect

describe("Future", () => {
  testAsync("sync chaining", finish =>
    Future.value("one")
    ->Future.map(s => s ++ "!")
    ->Future.get(s => s->expect->toEqual("one!")->finish)
  )

  testAsync("async chaining", finish =>
    Future.delay(25, () => 20)
    ->Future.map(s => string_of_int(s))
    ->Future.map(s => s ++ "!")
    ->Future.get(s => s->expect->toEqual("20!")->finish)
  )

  testAsync("tap", finish => {
    let v = ref(0)

    Future.value(99)
    ->Future.tap(n => v := n + 1)
    ->Future.map(n => n - 9)
    ->Future.get(n => (n, v.contents)->expect->toEqual((90, 100))->finish)
  })

  testAsync("flatMap", finish =>
    Future.value(59)
    ->Future.flatMap(n => Future.value(n + 1))
    ->Future.get(n => n->expect->toEqual(60)->finish)
  )

  testAsync("map2", finish =>
    Future.map2(Future.delay(20, () => 1), Future.value("a"), (num, str) => (num, str))->Future.get(
      tup => tup->expect->toEqual((1, "a"))->finish,
    )
  )

  testAsync("map5", finish =>
    Future.map5(
      Future.delay(20, () => 0),
      Future.delay(40, () => 1.2),
      Future.delay(60, () => "foo"),
      Future.delay(80, () => list{}),
      Future.delay(100, () => ()),
      (a, b, c, d, e) => (a, b, c, d, e),
    )->Future.get(tup => tup->expect->toEqual((0, 1.2, "foo", list{}, ()))->finish)
  )

  test("multiple gets", () => {
    let count = ref(0)
    let future = Future.make(
      resolve => {
        count := count.contents + 1
        resolve(count.contents)
      },
    )

    future->Future.get(_ => ())

    future->Future.get(_ => ())
    count.contents->expect->toBe(1)
  })

  testAsync("multiple gets (async)", finish => {
    let count = ref(0)
    let future = Future.sleep(25)->Future.map(() => 0)->Future.map(_ => count := count.contents + 1)

    future->Future.get(_ => ())

    let initialCount = count.contents

    future->Future.get(_ => ())

    future->Future.get(_ => (initialCount, count.contents)->expect->toEqual((0, 1))->finish)
  })

  testAsync("all (async)", finish =>
    {
      open Future
      all(list{value(1), delay(25, () => 2), delay(50, () => 3), sleep(75)->map(() => 4)})
    }->Future.get(
      result =>
        switch result {
        | list{1, 2, 3, 4} => pass->finish
        | _ => fail("Expected [1, 2, 3, 4]")->finish
        },
    )
  )
})

describe("Future Belt.Result", () => {
  let (\">>=", \"<$>") = {
    open Future
    (\">>=", \"<$>")
  }

  testAsync("mapOk 1", finish =>
    Belt.Result.Ok("two")
    ->Future.value
    ->Future.mapOk(s => s ++ "!")
    ->Future.get(r => Belt.Result.getExn(r)->expect->toEqual("two!")->finish)
  )

  testAsync("mapOk 2", finish =>
    Belt.Result.Error("err2")
    ->Future.value
    ->Future.mapOk(s => s ++ "!")
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err2")->finish
        },
    )
  )

  testAsync("mapError 1", finish =>
    Belt.Result.Ok("three")
    ->Future.value
    ->Future.mapError(s => s ++ "!")
    ->Future.get(r => Belt.Result.getExn(r)->expect->toEqual("three")->finish)
  )

  testAsync("mapError 2", finish =>
    Belt.Result.Error("err3")
    ->Future.value
    ->Future.mapError(s => s ++ "!")
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err3!")->finish
        },
    )
  )

  testAsync("flatMapOk 1", finish =>
    Belt.Result.Ok("four")
    ->Future.value
    ->Future.flatMapOk(s => Belt.Result.Ok(s ++ "!")->Future.value)
    ->Future.get(r => Belt.Result.getExn(r)->expect->toEqual("four!")->finish)
  )

  testAsync("flatMapOk 2", finish =>
    Belt.Result.Error("err4.1")
    ->Future.value
    ->Future.flatMapOk(s => Belt.Result.Ok(s ++ "!")->Future.value)
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err4.1")->finish
        },
    )
  )

  testAsync("flatMapOk 3", finish =>
    Belt.Result.Error("err4")
    ->Future.value
    ->Future.flatMapError(e => Belt.Result.Error(e ++ "!")->Future.value)
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err4!")->finish
        },
    )
  )

  testAsync("flatMapOkPure 1", finish =>
    Belt.Result.Ok("four")
    ->Future.value
    ->Future.flatMapOkPure(s => Belt.Result.Ok(s ++ "!"))
    ->Future.get(r => Belt.Result.getExn(r)->expect->toEqual("four!")->finish)
  )

  testAsync("flatMapOkPure 2", finish =>
    Belt.Result.Error("err4.1")
    ->Future.value
    ->Future.flatMapOkPure(s => Belt.Result.Ok(s ++ "!"))
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err4.1")->finish
        },
    )
  )

  testAsync("flatMapError 1", finish =>
    Belt.Result.Ok("five")
    ->Future.value
    ->Future.flatMapError(s => Belt.Result.Error(s ++ "!")->Future.value)
    ->Future.get(r => Belt.Result.getExn(r)->expect->toEqual("five")->finish)
  )

  testAsync("flatMapError 2", finish =>
    Belt.Result.Error("err5")
    ->Future.value
    ->Future.flatMapError(e => Belt.Result.Error(e ++ "!")->Future.value)
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("err5!")->finish
        },
    )
  )

  testAsync("mapOk5 success", finish =>
    Future.mapOk5(
      Future.value(Belt.Result.Ok(0)),
      Future.value(Belt.Result.Ok(1.1)),
      Future.value(Belt.Result.Ok("")),
      Future.value(Belt.Result.Ok(list{})),
      Future.value(Belt.Result.Ok(Some("x"))),
      (a, b, c, d, e) => (a, b, c, d, e),
    )->Future.get(
      r =>
        switch r {
        | Ok(tup) => tup->expect->toEqual((0, 1.1, "", list{}, Some("x")))->finish
        | Error(_) => fail("shouldn't be possible")->finish
        },
    )
  )

  testAsync("mapOk5 fails on first error", finish =>
    Future.mapOk5(
      Future.value(Belt.Result.Ok(0)),
      Future.delay(20, () => Belt.Result.Ok(1.)),
      Future.delay(10, () => Belt.Result.Error("first")),
      Future.value(Belt.Result.Ok("")),
      Future.delay(100, () => Belt.Result.Error("second")),
      (_, _, _, _, _) => None,
    )->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(x) => x->expect->toEqual("first")->finish
        },
    )
  )

  testAsync("tapOk 1", finish => {
    let x = ref(1)

    Belt.Result.Ok(10)
    ->Future.value
    ->Future.tapOk(n => x := x.contents + n)
    ->Future.get(_ => x.contents->expect->toEqual(11)->finish)
  })

  testAsync("tapOk 2", finish => {
    let y = ref(1)

    Belt.Result.Error(10)
    ->Future.value
    ->Future.tapOk(n => y := y.contents + n)
    ->Future.get(_ => y.contents->expect->toEqual(1)->finish)
  })

  testAsync("tapError 1", finish => {
    let x = ref(1)

    Belt.Result.Ok(10)
    ->Future.value
    ->Future.tapError(n => x := x.contents + n)
    ->Future.get(_ => x.contents->expect->toEqual(1)->finish)
  })

  testAsync("tapError", finish => {
    let y = ref(1)

    Belt.Result.Error(10)
    ->Future.value
    ->Future.tapError(n => y := y.contents + n)
    ->Future.get(_ => y.contents->expect->toEqual(11)->finish)
  })
  testAsync("<$> 1", finish =>
    \"<$>"(
      \"<$>"(Future.value(Belt.Result.Ok("infix ops")), x => x ++ " "),
      x => x ++ "rock!",
    )->Future.get(r => r->Belt.Result.getExn->expect->toEqual("infix ops rock!")->finish)
  )

  testAsync("<$> 2", finish =>
    \"<$>"(
      \"<$>"(Future.value(Belt.Result.Error("infix ops")), x => x ++ " "),
      x => x ++ "suck!",
    )->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(e) => e->expect->toEqual("infix ops")->finish
        },
    )
  )

  testAsync(">>= 1", finish => {
    let appendToString = (appendedString, s) => (s ++ appendedString)->Belt.Result.Ok->Future.value

    \">>="(
      Future.value(Belt.Result.Ok("infix ops")),
      s => appendToString(" still rock!", s),
    )->Future.get(r => r->expect->toEqual(Belt.Result.Ok("infix ops still rock!"))->finish)
  })

  testAsync("value recursion is stack safe", finish => {
    let next = x => Future.value(~executor=#trampoline, x + 1)
    let numberOfLoops = 10000
    let rec loop = x =>
      next(x)
      ->Future.flatMap(x => Future.value(x + 1))
      ->Future.map(x => x + 1)
      ->Future.flatMap(x => Future.value(x + 1)->Future.flatMap(x => Future.value(x + 1)))
      ->Future.flatMap(
        x' =>
          if x' > numberOfLoops {
            Future.value(x')
          } else {
            loop(x')
          },
      )
    loop(0)->Future.get(r => r->expect->toBeGreaterThan(numberOfLoops)->finish)
  })

  testAsync("async recursion is stack safe", finish => {
    let next = x => Future.delay(~executor=#trampoline, 1, () => x + 1)
    let numberOfLoops = 1000
    let rec loop = x =>
      next(x)->Future.flatMap(
        x' =>
          if x' == numberOfLoops {
            Future.value(x')
          } else {
            loop(x')
          },
      )
    loop(0)->Future.get(r => r->expect->toEqual(numberOfLoops)->finish)
  })
})
