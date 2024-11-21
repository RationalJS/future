open Jest
open Expect

exception TestError(string)

describe("FutureJs", () => {
  let errorTransformer = x => x

  testAsync("fromPromise (resolved)", finish =>
    Js.Promise.resolve(42)
    ->FutureJs.fromPromise(errorTransformer)
    ->Future.get(r => Belt.Result.getExn(r) |> expect |> toEqual(42) |> finish)
  )

  testAsync("fromPromise (rejected)", finish => {
    let err = TestError("oops!")
    Js.Promise.reject(err)
    ->FutureJs.fromPromise(errorTransformer)
    ->Future.get(r =>
      switch r {
      | Belt.Result.Ok(_) => fail("shouldn't be possible") |> finish
      | Belt.Result.Error(_) => pass |> finish
      }
    )
  })

  testAsync("fromPromise (internal rejection)", finish => {
    let err = TestError("boom!")
    let promise = Js.Promise.make((~resolve as _, ~reject) => reject(. err))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(r =>
      switch r {
      | Belt.Result.Ok(_) => fail("shouldn't be possible") |> finish
      | Belt.Result.Error(_) => pass |> finish
      }
    )
  })

  testAsync("fromPromise (internal exception)", finish => {
    let err = TestError("explode!")
    let promise = Js.Promise.make((~resolve as _, ~reject as _) => raise(err))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(r =>
      switch r {
      | Belt.Result.Ok(_) => fail("shouldn't be possible") |> finish
      | Belt.Result.Error(_) => pass |> finish
      }
    )
  })

  testAsync("fromPromise (wrapped callback)", finish => {
    let err = TestError("throwback!")
    let nodeFn = callback => callback(err)
    let promise = Js.Promise.make((~resolve as _, ~reject) => nodeFn(err => reject(. err)))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(r =>
      switch r {
      | Belt.Result.Ok(_) => fail("shouldn't be possible") |> finish
      | Belt.Result.Error(_) => pass |> finish
      }
    )
  })

  testAsync("fromPromise (layered failure)", finish => {
    let err = TestError("confused!")
    let nodeFn = callback => callback(err)
    let promise = Js.Promise.make((~resolve as _, ~reject) => nodeFn(err => reject(. err)))
    let future = () => FutureJs.fromPromise(promise, errorTransformer)

    Future.value(Belt.Result.Ok("ignored"))
    ->Future.flatMapOk(_ => future())
    ->Future.get(r =>
      switch r {
      | Belt.Result.Ok(_) => fail("shouldn't be possible") |> finish
      | Belt.Result.Error(_) => pass |> finish
      }
    )
  })
  testPromise("toPromise", () =>
    Future.delay(5, () => "payload")
    |> FutureJs.toPromise
    |> Js.Promise.catch(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.then_(x => Js.Promise.resolve(x |> expect |> toEqual("payload")))
  )

  testPromise("resultToPromise (Ok result)", () =>
    Future.delay(5, () => Belt.Result.Ok("payload"))
    |> FutureJs.resultToPromise
    |> Js.Promise.catch(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.then_(x => Js.Promise.resolve(x |> expect |> toEqual("payload")))
  )

  testPromise("resultToPromise (Error exn)", () => {
    let err = TestError("error!")
    Future.delay(5, () => Belt.Result.Error(err))
    |> FutureJs.resultToPromise
    |> Js.Promise.then_(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.catch(_ => Js.Promise.resolve(pass)) // Going from Future to Js.Promise loses information...
  })

  testPromise("resultToPromise (Error `PolymorphicVariant)", () => {
    let err = #TestError
    Future.delay(5, () => Belt.Result.Error(err))
    |> FutureJs.resultToPromise
    |> Js.Promise.then_(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.catch(_ => Js.Promise.resolve(pass)) // Going from Future to Js.Promise loses information...
  })
})
