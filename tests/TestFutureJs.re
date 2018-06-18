open BsOspec.Cjs;
exception TestError(string);

describe("FutureJs", () => {
  let errorTransformer = x => x;

  testAsync("fromPromise (resolved)", done_ => {
    Js.Promise.resolve(42)
    |. FutureJs.fromPromise(errorTransformer)
    |. Future.get(r => {
      Belt.Result.getExn(r) |. equals(42);
      done_();
    });
  });

  testAsync("fromPromise (rejected)", done_ => {
    let err = TestError("oops!");
    Js.Promise.reject(err)
    |. FutureJs.fromPromise(errorTransformer)
    |. Future.get(r =>
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error(e) => e |. equals(err)
      }
      |> _ => done_()
      );
  });

  testAsync("fromPromise (internal rejection)", done_ => {
    let err = TestError("boom!");
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      reject(. err);
    });

    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((e)) => e |. equals(err)
      };

      done_();
    });
  });

  testAsync("fromPromise (internal exception)", done_ => {
    let err = TestError("explode!");
    let promise = Js.Promise.make((~resolve as _, ~reject as _) => {
      raise(err);
    });

    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(err)
      };

      done_();
    });
  });

  testAsync("fromPromise (wrapped callback)", done_ => {
    let err = TestError("throwback!");
    let nodeFn = (callback) => callback(err);
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      nodeFn((err) => reject(. err));
    });
    
    FutureJs.fromPromise(promise, errorTransformer)
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(err)
      };

      done_();
    });
  });

  testAsync("fromPromise (layered failure)", done_ => {
    let err = TestError("confused!");
    let nodeFn = (callback) => callback(err);
    let promise = Js.Promise.make((~resolve as _, ~reject) => {
      nodeFn((err) => reject(. err));
    });
    let future = () => FutureJs.fromPromise(promise, errorTransformer);
    
    Future.value(Belt.Result.Ok("ignored"))
    |. Future.flatMapOk(_ => future())
    |. Future.get(r => {
      switch(r) {
      | Belt.Result.Ok(_) => raise(TestError("shouldn't be possible"))
      | Belt.Result.Error((s)) => s |. equals(err)
      };

      done_();
    });
  })
});
