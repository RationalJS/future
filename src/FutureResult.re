let deprecate = (ft, name, more) =>
  ft
  ->Future.tap(_ =>
      Js.Console.warn("FutureResult." ++ name ++ " is deprecated." ++ more)
    );

let mapOk = (future, f) =>
  future
  ->Future.map(r =>
      switch (r) {
      | Js.Result.Ok(v) => f(v)
      | Error(e) => Js.Result.Error(e)
      }
    )
  ->deprecate("mapOk", " Please user Future.mapOk instead.");

let flatMapOk = (future, f) =>
  future
  ->Future.flatMap(r =>
      switch (r) {
      | Js.Result.Ok(v) => f(v)
      | Error(e) => Future.value(Js.Result.Error(e))
      }
    )
  ->deprecate("flatMapOk", "");

let mapError = (future, f) =>
  future
  ->Future.map(r =>
      switch (r) {
      | Js.Result.Error(v) => f(v)
      | Ok(a) => Js.Result.Ok(a)
      }
    )
  ->deprecate("mapError", " Please user Future.mapError instead.");

let flatMapError = (future, f) =>
  future
  ->Future.flatMap(r =>
      switch (r) {
      | Js.Result.Error(v) => f(v)
      | Ok(a) => Future.value(Js.Result.Ok(a))
      }
    )
  ->deprecate("mapOk", "");

let tapOk = (future, f) =>
  future
  ->Future.tap(r =>
      switch (r) {
      | Js.Result.Ok(v) => f(v)->ignore
      | Error(_) => ()
      }
    )
  ->deprecate("tapOk", " Please user Future.tapOk instead.");

let tapError = (future, f) =>
  future
  ->Future.tap(r =>
      switch (r) {
      | Js.Result.Error(v) => f(v)->ignore
      | Ok(_) => ()
      }
    )
  ->deprecate("tapError", " Please user Future.tapError instead.");
