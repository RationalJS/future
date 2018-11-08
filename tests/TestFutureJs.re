open BsOspec.Cjs;
exception TestError(string);

type timeoutId;
[@bs.val] [@bs.val] external setTimeout : ([@bs.uncurry] (unit => unit), int) => timeoutId = "";

describe("FutureJs", () => {
  let errorTransformer = x => x;

  let delay = (ms, f) => Future.make(resolve =>
    setTimeout(() => f() |> resolve, ms) |> ignore
  );

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
  });

  let checkPromisedValue = (done_, expected, actual) => {
    equals(expected, actual);
    done_();
    Js.Promise.resolve(());
  };

  testAsync("toPromise (trivial Ok result)", done_ => {
    Future.value(Belt.Result.Ok("payload"))
    |> FutureJs.toPromise
    |> Js.Promise.catch(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.then_(checkPromisedValue(done_, "payload"));
  });

  testAsync("toPromise (trivial Error result)", done_ => {
    let err = TestError("error!");

    Future.value(Belt.Result.Error(err))
    |> FutureJs.toPromise
    |> Js.Promise.then_(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.catch(checkPromisedValue(done_, err));
  });

  testAsync("toPromise (Ok result from delayed future)", done_ => {
    delay(5, () => Belt.Result.Ok("payload"))
    |> FutureJs.toPromise
    |> Js.Promise.catch(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.then_(checkPromisedValue(done_, "payload"));
  });

  testAsync("toPromise (Error result from delayed future)", done_ => {
    let err = TestError("error!");

    delay(5, () => Belt.Result.Error(err))
    |> FutureJs.toPromise
    |> Js.Promise.then_(_ => raise(TestError("shouldn't be possible")))
    |> Js.Promise.catch(checkPromisedValue(done_, err));
  });

});
