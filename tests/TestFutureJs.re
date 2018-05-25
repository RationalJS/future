open BsOspec.Cjs;
exception TestError(string);

describe("FutureJs", () => {
  let errorTransformer = Js.String.make;

  testAsync("fromPromise (resolved)", done_ => {
    Js.Promise.resolve(42)
    |. FutureJs.fromPromise(errorTransformer)
    |. Future.get(r => {
      Belt.Result.getExn(r) |. equals(42);
      done_();
    });
  });

  testAsync("fromPromise (rejected)", done_ => {
    let expected = "TestFutureJs.TestError,2,oops!";
    Js.Promise.reject(TestError("oops!"))
    |. FutureJs.fromPromise(errorTransformer)
    |. Future.get(r =>
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error(s) => s |. equals(expected)
      }
      |> _ => done_()
      );
  });
});
