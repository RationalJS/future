let deprecate = (ft, name, more) =>
  ft->Future.tap(_ => Console.warn("FutureResult." ++ (name ++ (" is deprecated." ++ more))))

let mapOk = (future, f) =>
  future
  ->Future.map(r =>
    switch r {
    | Ok(v) => f(v)
    | Error(e) => Error(e)
    }
  )
  ->deprecate("mapOk", " Please use Future.mapOk instead.")

let flatMapOk = (future, f) =>
  future
  ->Future.flatMap(r =>
    switch r {
    | Ok(v) => f(v)
    | Error(e) => Future.value(Error(e))
    }
  )
  ->deprecate("flatMapOk", "")

let mapError = (future, f) =>
  future
  ->Future.map(r =>
    switch r {
    | Error(v) => f(v)
    | Ok(a) => Ok(a)
    }
  )
  ->deprecate("mapError", " Please use Future.mapError instead.")

let flatMapError = (future, f) =>
  future
  ->Future.flatMap(r =>
    switch r {
    | Error(v) => f(v)
    | Ok(a) => Future.value(Ok(a))
    }
  )
  ->deprecate("mapOk", "")

let tapOk = (future, f) =>
  future
  ->Future.tap(r =>
    switch r {
    | Ok(v) => f(v)->ignore
    | Error(_) => ()
    }
  )
  ->deprecate("tapOk", " Please use Future.tapOk instead.")

let tapError = (future, f) =>
  future
  ->Future.tap(r =>
    switch r {
    | Error(v) => f(v)->ignore
    | Ok(_) => ()
    }
  )
  ->deprecate("tapError", " Please use Future.tapError instead.")
