open Jest
open Expect

exception TestError(string)

describe("FutureJs", () => {
  let errorTransformer = x => x

  testAsync("fromPromise (resolved)", finish =>
    Promise.resolve(42)
    ->FutureJs.fromPromise(errorTransformer)
    ->Future.get(r => Result.getExn(r)->expect->toEqual(42)->finish)
  )

  testAsync("fromPromise (rejected)", finish => {
    let err = TestError("oops!")
    Promise.reject(err)
    ->FutureJs.fromPromise(errorTransformer)
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(_) => pass->finish
        },
    )
  })

  testAsync("fromPromise (internal rejection)", finish => {
    let err = TestError("boom!")
    let promise = Promise.make((_, reject) => reject(err))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(_) => pass->finish
        },
    )
  })

  testAsync("fromPromise (internal exception)", finish => {
    let err = TestError("explode!")
    let promise = Promise.make((_, _) => raise(err))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(_) => pass->finish
        },
    )
  })

  testAsync("fromPromise (wrapped callback)", finish => {
    let err = TestError("throwback!")
    let nodeFn = callback => callback(err)
    let promise = Promise.make((_, reject) => nodeFn(err => reject(err)))

    FutureJs.fromPromise(promise, errorTransformer)->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(_) => pass->finish
        },
    )
  })

  testAsync("fromPromise (layered failure)", finish => {
    let err = TestError("confused!")
    let nodeFn = callback => callback(err)
    let promise = Promise.make((_, reject) => nodeFn(err => reject(err)))
    let future = () => FutureJs.fromPromise(promise, errorTransformer)

    Future.value(Ok("ignored"))
    ->Future.flatMapOk(_ => future())
    ->Future.get(
      r =>
        switch r {
        | Ok(_) => fail("shouldn't be possible")->finish
        | Error(_) => pass->finish
        },
    )
  })
  testPromise("toPromise", () =>
    Future.delay(5, () => "payload")
    ->FutureJs.toPromise
    ->Promise.then(x => Promise.resolve(x->expect->toEqual("payload")))
    ->Promise.catch(_ => raise(TestError("shouldn't be possible")))
  )

  testPromise("resultToPromise (Ok result)", () =>
    FutureJs.resultToPromise(Future.delay(5, () => Ok("payload")))
    ->Promise.then(x => Promise.resolve(x->expect->toEqual("payload")))
    ->Promise.catch(_ => raise(TestError("shouldn't be possible")))
  )

  testPromise("resultToPromise (Error exn)", () => {
    let err = TestError("error!")

    FutureJs.resultToPromise(Future.delay(5, () => Error(err)))
    ->Promise.then(_ => raise(TestError("shouldn't be possible")))
    ->Promise.catch(_ => Promise.resolve(pass)) // Going from Future to Promise loses information...
  })

  testPromise("resultToPromise (Error `PolymorphicVariant)", () => {
    let err = #TestError

    FutureJs.resultToPromise(Future.delay(5, () => Error(err)))
    ->Promise.then(_ => raise(TestError("shouldn't be possible")))
    ->Promise.catch(_ => Promise.resolve(pass)) // Going from Future to Promise loses information...
  })
})
