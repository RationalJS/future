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

  testAsync("fromPromise (internal rejection)", done_ => {
    let expected = "TestFutureJs.TestError,2,boom!";
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      reject(. TestError("boom!"));
    });

    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(expected)
      };

      done_();
    });
  });

  testAsync("fromPromise (internal exception)", done_ => {
    let expected = "TestFutureJs.TestError,2,explode!";
    let promise = Js.Promise.make((~resolve as _, ~reject as _) => {
      raise(TestError("explode!"));
    });

    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(expected)
      };

      done_();
    });
  });

  testAsync("fromPromise (wrapped callback)", done_ => {
    let expected = "TestFutureJs.TestError,2,throwback!";
    let nodeFn = (callback) => callback(TestError("throwback!"));
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      nodeFn((err) => reject(. err));
    });
    
    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(expected)
      };

      done_();
    });
  });

  testAsync("fromPromise (layered failure)", done_ => {
    let expected = "TestFutureJs.TestError,2,confused!";
    let nodeFn = (callback) => callback(TestError("confused!"));
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      nodeFn((err) => reject(. err));
    });
    let future = () => FutureJs.fromPromise(promise, errorTransformer);
    
    Future.value(Belt.Result.Ok("ignored"))
    |. Future.flatMapOk(_ => future())
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(expected)
      };

      done_();
    });
  });

  testAsync("fromPromise (exception in callback)", done_ => {

    let expected = "TestFutureJs.TestError,2,confused!";
    let nodeFn = (callback) => callback(TestError("confused!"));
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      nodeFn((err) => reject(. err));
    });
    let future = () => FutureJs.fromPromise(promise, errorTransformer);
    
    Future.value(Belt.Result.Ok("ignored"))
    |. Future.tap(_ => Js.log("huh? - 1"))
    |. Future.mapOk(_ => raise(TestError("oh the noes!")))
    |. Future.tap(_ => Js.log("huh? - 2"))
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(expected)
      };
      done_();
    });
  });
});
